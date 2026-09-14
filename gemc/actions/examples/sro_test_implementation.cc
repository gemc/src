#include <gemc/gstreamer/sro/gSROImplementation.h>
#include <gemc/gdynamicDigitization/gdynamicdigitization.h>

#include <fstream>
#include <thread>

namespace {
struct Data final : GSROData {
	GSROEventId event;
	std::size_t worker;
	Data(GSROEventId id, std::size_t thread) : event(id), worker(thread) {}
	std::size_t size_bytes() const noexcept override { return sizeof(*this); }
};

struct Timing final : GSROTiming {
	std::optional<GSROTime> earliest_remaining_time(GSROEventId id) const override {
		return GSROTime{static_cast<std::int64_t>(id) * 10};
	}
};

struct Sink final : GSROFrameSink {
	std::ofstream file;
	const std::thread::id owner = std::this_thread::get_id();
	explicit Sink(const std::string& filename) {
		file.exceptions(std::ios::failbit | std::ios::badbit);
		file.open(filename);
	}
	void write_frame(GSROFrame frame) override {
		if (owner != std::this_thread::get_id()) { throw std::runtime_error("Wrong SRO output thread"); }
		const auto& data = dynamic_cast<const Data&>(*frame.data);
		file << "F " << data.event << ' ' << frame.frame_id << ' ' << frame.begin.count()
		     << ' ' << data.worker << '\n';
	}
	void finish_output() override { file.close(); }
};

struct Crate final : GSROCratePlugin {
	GSROCrateId id;
	std::vector<GSROPayload> pending;
	Crate(GSROFrameSink& sink, GSROCrateId crate) : GSROCratePlugin(sink), id(crate) {}
	void consume_payload(GSROPayload payload) override { pending.push_back(std::move(payload)); }
	void advance_time(GSROTime boundary) override {
		for (auto& payload : pending) {
			if (payload.time >= boundary) { throw std::runtime_error("Unproven SRO payload released"); }
			output.write_frame({id, payload.sequence, payload.time, payload.time + GSROTime{1},
			                    std::move(payload.data)});
		}
		pending.clear();
	}
	void finish_run(const GSROEndContext& context) override {
		pending.clear(); // This test implementation discards the unproven tail.
		auto& sink = dynamic_cast<Sink&>(output);
		sink.file << "E " << (context.reason == GSROEndReason::completed ? "completed" : "interrupted")
		          << ' ' << (context.safe_time ? context.safe_time->count() : -1) << '\n';
	}
};

class Implementation final : public GSROImplementation {
public:
	using GSROImplementation::GSROImplementation;
	GSROConfiguration configure_run(const GSRORunContext&) override {
		GSROConfiguration config;
		config.timing = std::make_shared<Timing>();
		config.crate_limits.queue_messages = 2;
		return config;
	}
	GSROCrateResources create_crate(GSROCrateId id, const GSRORunContext& run) const override {
		auto sink = std::make_unique<Sink>(run.output_basename + "_r" + std::to_string(run.run_id) +
		                                   "_c" + std::to_string(id) + ".txt");
		auto crate = std::make_unique<Crate>(*sink, id);
		return {std::move(sink), std::move(crate)};
	}
};

class Digitizer final : public GDynamicDigitization {
public:
	using GDynamicDigitization::GDynamicDigitization;
	bool defineReadoutSpecsImpl() override { return true; } // Test hits are injected without step processing.
	std::unique_ptr<GTrueInfoData> collectTrueInformationImpl(GHit*, std::size_t) override { return nullptr; }
	void stream_hit(GHit*, std::size_t, const GSROEventContext& event, const GSROEmit& emit) const override {
		if (gopts->getSwitch("sro_test_worker_failure")) {
			throw std::runtime_error("synthetic SRO worker failure");
		}
		const auto worker = std::hash<std::thread::id>{}(std::this_thread::get_id());
		for (GSROCrateId crate : {1u, 2u}) {
			emit(crate, GSROTime{static_cast<std::int64_t>(event.event_id) * 10},
			     std::make_unique<Data>(event.event_id, worker));
		}
	}
};
} // namespace

extern "C" GSROImplementation* GSROImplementationFactory(const std::shared_ptr<GOptions>& options) {
	return new Implementation(options);
}
extern "C" GDynamicDigitization* GDynamicDigitizationFactory(const std::shared_ptr<GOptions>& options) {
	return new Digitizer(options);
}
