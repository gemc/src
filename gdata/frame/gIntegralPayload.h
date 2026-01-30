#pragma once

/**
 * \file gIntegralPayload.h
 * \brief Defines \ref GIntegralPayload one integrated electronics payload.
 *
 * \details
 * A payload is the minimal set of fields typically needed to represent a readout sample:
 * - crate / slot / channel : readout address
 * - charge                : integrated amplitude (or ADC proxy)
 * - time                  : time coordinate (or TDC proxy)
 */

#include "glogger.h"
#include <vector>

struct GIntegralPayload {
public:
	/**
	 * \brief Construct an integral payload.
	 *
	 * \param c      Crate number (readout address component).
	 * \param s      Slot number (module position within the crate).
	 * \param ch     Channel number within the slot/module.
	 * \param q      Integrated charge (or ADC proxy).
	 * \param t      Time value (or TDC proxy).
	 * \param logger Logger instance used for debug traces.
	 */
	GIntegralPayload(int c, int s, int ch, int q, int t, std::shared_ptr<GLogger> logger)
		: log(logger), crate(c), slot(s), channel(ch), charge(q), time(t) {
		log->debug(CONSTRUCTOR, "GIntegralPayload crate ", crate, " slot ", slot, " channel ", channel, " charge ",
		           charge, " time ", time);
	}

	/**
	 * \brief Destructor (logs for debug builds/configurations).
	 */
	~GIntegralPayload() {
		log->debug(DESTRUCTOR, "GIntegralPayload crate ", crate, " slot ", slot, " channel ", channel, " charge ",
		           charge, " time ", time);
	}

	/**
	 * \brief Export payload as a fixed-order vector of integers.
	 *
	 * Order is:
	 * 0) crate
	 * 1) slot
	 * 2) channel
	 * 3) charge
	 * 4) time
	 *
	 * This ordering matches \ref GFrameDataCollection::addIntegralPayload().
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
	std::shared_ptr<GLogger> log; ///< Logger instance (diagnostics only).
	int crate;   ///< Crate number.
	int slot;    ///< Slot number.
	int channel; ///< Channel number.
	int charge;  ///< Integrated charge / ADC proxy.
	int time;    ///< Time / TDC proxy.
};
