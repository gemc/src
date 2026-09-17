#include "sro_payload.h"

#include <gemc/gdynamicDigitization/gdynamicdigitization.h>
#include <gemc/gstreamer/sro/gSROImplementation.h>
#include <gemc/gparticle/gparticle_options.h>

#include <fstream>
#include <cmath>
#include <map>
#include <random>

namespace simple_sro {

// 1. Worker: build an owned electronics sample and dispatch it immediately.
class Digitizer final : public GDynamicDigitization {
public:
	using GDynamicDigitization::GDynamicDigitization;
	bool loadTTImpl([[maybe_unused]] int runno, [[maybe_unused]] const std::string& variation) override {
		// GEMC calls this before the run. This example's mapping is independent of run/variation.
		// Keys match the sensitive IDs in sro.py; only this table defines the hardware addresses.
		auto table = std::make_shared<GTranslationTable>(gopts);
		using Mode = GElectronic::ComparisonMode;
		table->addGElectronicWithIdentity({101}, GElectronic(1, 3, 0, Mode::crate_slot_channel));
		table->addGElectronicWithIdentity({102}, GElectronic(1, 3, 1, Mode::crate_slot_channel));
		table->addGElectronicWithIdentity({103}, GElectronic(2, 3, 0, Mode::crate_slot_channel));
		table->addGElectronicWithIdentity({104}, GElectronic(2, 3, 1, Mode::crate_slot_channel));
		// Publish through the inherited member; workers only read the completed table during the run.
		translationTable = std::move(table);
		return true;
	}
	bool defineReadoutSpecsImpl() override {
		// Group steps within a plane into one hit. This Geant4 hit window is independent of SRO frames.
		readoutSpecs = std::make_shared<GReadoutSpecs>(1000 * CLHEP::ns, 0, 1 * CLHEP::cm, log);
		return true;
	}
	void stream_hit(GHit* hit, std::size_t, const GSROEventContext& event,
	                const GSROEmit& emit) const override {
		if (!translationTable) { throw std::logic_error("SRO translation table was not loaded"); }
		// getTTID() returns the sensitive identity vector. Missing entries fail in getElectronics().
		const auto address = translationTable->getElectronics(hit->getTTID()).getHAddress();
		AdcSample sample{static_cast<std::uint32_t>(address[0]), static_cast<std::uint32_t>(address[1]),
		                 static_cast<std::uint32_t>(address[2]), 0};
		// Local RNG, seeded from the address and event: reproducible across worker schedules, with no
		// shared random state and no changes to the simulation's physics RNG. This is a toy ADC model.
		std::seed_seq seed{12345u, static_cast<std::uint32_t>(event.event_id),
		                   sample.crate, sample.slot, sample.channel};
		std::mt19937 random(seed);
		sample.adc = std::uniform_int_distribution<std::uint32_t>(0, 4095)(random);
		const auto event_width = gparticle::getEventTimeWidth(gopts);
		if (event_width <= 0 || std::floor(event_width) != event_width) {
			throw std::invalid_argument("SRO example requires a positive integral eventTimeWidth");
		}
		const auto time = GSROTime{static_cast<std::int64_t>(event_width)} *
		                  static_cast<std::int64_t>(event.event_id);
		emit(sample.crate, time, std::make_unique<AdcPayload>(sample));
	}
};

// 2. Progress: a bound for ALL events at or after the first event not yet fully delivered.
class Timing final : public GSROTiming {
public:
	explicit Timing(GSROTime width) : width(width) {}
	std::optional<GSROTime> earliest_remaining_time(GSROEventId first) const override {
		return width * static_cast<std::int64_t>(first);
	}

private:
	GSROTime width;
};

// 3. Output: one CSV file per crate/run, opened, written, and closed on its crate thread.
class CsvSink final : public GSROFrameSink {
public:
	explicit CsvSink(const std::string& filename) {
		file.exceptions(std::ios::failbit | std::ios::badbit); // Report open/write/close failures to GEMC.
		file.open(filename);
		file << "frame_id,begin_ns,end_ns,crate,slot,channel,adc\n";
	}
	void write_frame(GSROFrame frame) override {
		const auto& data = dynamic_cast<const AdcFrame&>(*frame.data);
		for (const auto& sample : data.samples) {
			file << frame.frame_id << ',' << frame.begin.count() << ',' << frame.end.count() << ','
			     << sample.crate << ',' << sample.slot << ',' << sample.channel << ',' << sample.adc << '\n';
		}
	}
	void finish_output() override { file.close(); }
private:
	std::ofstream file;
};

// 4. Crate: assemble frames from every channel/worker; only proven complete frames can be written.
class Crate final : public GSROCratePlugin {
public:
	Crate(GSROFrameSink& sink, GSROCrateId id) : GSROCratePlugin(sink), crate_id(id) {}
	void consume_payload(GSROPayload payload) override {
		const auto& sample = dynamic_cast<const AdcPayload&>(*payload.data).sample;
		if (sample.crate != crate_id || payload.time < GSROTime{0}) {
			throw std::invalid_argument("Unexpected crate or negative time in the simple SRO example");
		}
		const auto frame_id = static_cast<std::uint64_t>(payload.time / frame_duration);
		auto& frame = frames[frame_id];
		if (!frame) { frame = std::make_unique<AdcFrame>(); }
		frame->samples.push_back(sample);
	}
	void advance_time(GSROTime safe_time) override {
		while (!frames.empty()) {
			const auto id = frames.begin()->first;
			const auto begin = frame_duration * static_cast<std::int64_t>(id);
			const auto end = begin + frame_duration;
			if (end > safe_time) { break; } // A payload exactly at end belongs to the NEXT frame.
			auto frame = frames.extract(frames.begin());
			output.write_frame({crate_id, id, begin, end, std::move(frame.mapped())});
		}
	}
	void finish_run(const GSROEndContext&) override {
		// GEMC has delivered the remaining input, but it has not declared that input complete.
		// This implementation discards unfinished frames, including any unproven tail after interruption.
		frames.clear();
	}
private:
	GSROCrateId crate_id;
	std::map<std::uint64_t, std::unique_ptr<AdcFrame>> frames;
};

// The same library exports both the worker digitizer and the independent crate/timing implementation.
class Implementation final : public GSROImplementation {
public:
	using GSROImplementation::GSROImplementation;
	GSROConfiguration configure_run(const GSRORunContext&) override {
		const auto width = gparticle::getEventTimeWidth(options);
		if (width <= 0 || std::floor(width) != width) {
			throw std::invalid_argument("SRO example requires a positive integral eventTimeWidth");
		}
		return {std::make_shared<Timing>(GSROTime{static_cast<std::int64_t>(width)}), {}, 65536};
	}
	GSROCrateResources create_crate(GSROCrateId id, const GSRORunContext& run) const override {
		auto sink = std::make_unique<CsvSink>(run.output_basename + "_r" + std::to_string(run.run_id) +
		                                      "_crate" + std::to_string(id) + ".csv");
		auto crate = std::make_unique<Crate>(*sink, id);
		return {std::move(sink), std::move(crate)};
	}
};

} // namespace simple_sro

extern "C" GDynamicDigitization* GDynamicDigitizationFactory(const std::shared_ptr<GOptions>& options) {
	return new simple_sro::Digitizer(options);
}
extern "C" GSROImplementation* GSROImplementationFactory(const std::shared_ptr<GOptions>& options) {
	return new simple_sro::Implementation(options);
}
