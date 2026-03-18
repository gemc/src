#pragma once

/**
 * \file gIntegralPayload.h
 * \brief Defines GIntegralPayload, one integrated electronics payload sample.
 *
 * \details
 * A payload is the minimal set of fields typically needed to represent a readout sample:
 * - crate / slot / channel : readout address or hardware coordinates
 * - charge                 : integrated amplitude or ADC proxy
 * - time                   : time coordinate or TDC proxy
 *
 * This struct is used by GFrameDataCollection to store per-frame integrated samples.
 *
 * \note
 * This class is intentionally lightweight and stores values as integers. Interpretation, units,
 * calibration, and mapping to hardware are the responsibility of higher-level producers/consumers.
 */

#include "glogger.h"
#include <vector>

/**
 * \defgroup gdata_integral_payload GData integral payload
 * \brief Fixed-layout frame payload representing one integrated readout sample.
 *
 * \details
 * This topic documents the compact payload object stored inside frame collections. The payload is
 * intentionally small and ordered so it can be exported as a packed integer vector when needed.
 */

/**
 * \brief Stores one integrated readout payload sample.
 * \ingroup gdata_integral_payload
 */
struct GIntegralPayload
{
public:
	/**
	 * \brief Construct an integral payload.
	 *
	 * \details
	 * Stores the provided values and emits a debug log message.
	 * Logging is intended for diagnostics in examples/tests and can be controlled through logger configuration.
	 *
	 * \param c      Crate number, the first readout-address component.
	 * \param s      Slot number, the module position within the crate.
	 * \param ch     Channel number within the slot or module.
	 * \param q      Integrated charge or ADC proxy.
	 * \param t      Time value or TDC proxy.
	 * \param logger Logger instance used for debug traces.
	 */
	GIntegralPayload(int c, int s, int ch, int q, int t, std::shared_ptr<GLogger> logger)
		: log(logger), crate(c), slot(s), channel(ch), charge(q), time(t) {
		log->debug(CONSTRUCTOR, "GIntegralPayload crate ", crate, " slot ", slot, " channel ", channel, " charge ",
		           charge, " time ", time);
	}

	/**
	 * \brief Destructor.
	 *
	 * \details
	 * The payload owns no external resources.
	 * Destruction logging is useful mainly for lifecycle tracing in tests and examples.
	 */
	~GIntegralPayload() {
		log->debug(DESTRUCTOR, "GIntegralPayload crate ", crate, " slot ", slot, " channel ", channel, " charge ",
		           charge, " time ", time);
	}

	/**
	 * \brief Exports the payload as a fixed-order vector of integers.
	 *
	 * \details
	 * Order is:
	 * - element 0 : crate
	 * - element 1 : slot
	 * - element 2 : channel
	 * - element 3 : charge
	 * - element 4 : time
	 *
	 * This ordering matches the expectation in
	 * \ref GFrameDataCollection::addIntegralPayload "addIntegralPayload()".
	 *
	 * \return Vector \c {crate, slot, channel, charge, time}.
	 */
	[[nodiscard]] std::vector<int> getPayload() const {
		std::vector<int> payload;
		payload.push_back(crate);
		payload.push_back(slot);
		payload.push_back(channel);
		payload.push_back(charge);
		payload.push_back(time);
		return payload;
	}

private:
	std::shared_ptr<GLogger> log;     ///< Logger instance used only for diagnostics.
	int                      crate;   ///< Crate number.
	int                      slot;    ///< Slot number inside the crate.
	int                      channel; ///< Channel number inside the slot or module.
	int                      charge;  ///< Integrated charge or ADC-like value.
	int                      time;    ///< Time coordinate or TDC-like value.
};