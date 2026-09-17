#pragma once

#include <gemc/gstreamer/sro/gSROData.h>
#include <vector>

namespace simple_sro {

// The entire example record: an electronics address and a 12-bit ADC count. These are values,
// not pointers into a hit or a digitizer. Crate is repeated in the transport envelope for routing.
// The worker obtains crate/slot/channel from the translation table populated by loadTTImpl.
struct AdcSample {
	std::uint32_t crate;   // 1 or 2 in this geometry.
	std::uint32_t slot;    // 3 in this geometry.
	std::uint32_t channel; // 0 or 1 in each crate.
	std::uint32_t adc;     // Uniform random integer in [0, 4095]; no energy calibration.
};

// A worker transfers one sample to its crate. GEMC supplies event_id and sequence in GSROPayload;
// its time field is separate from this experiment-defined record.
struct AdcPayload final : GSROData {
	explicit AdcPayload(AdcSample value) : sample(value) {}
	AdcSample sample;
	std::size_t size_bytes() const noexcept override { return sizeof(*this); }
};

// The crate combines samples from every worker/channel into one time frame. Only the crate thread
// modifies this vector. The sink receives ownership after the frame is proven complete.
struct AdcFrame final : GSROData {
	std::vector<AdcSample> samples;
	std::size_t size_bytes() const noexcept override {
		return sizeof(*this) + samples.capacity() * sizeof(AdcSample);
	}
};

// Deliberately artificial timing: every event samples all four channels at event_id * eventTimeWidth.
// The example YAML sets eventTimeWidth to 10 ns, so four events fit in each 40 ns frame.
inline constexpr GSROTime frame_duration{40};

} // namespace simple_sro
