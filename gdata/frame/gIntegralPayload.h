#ifndef GINTEGRALPAYLOAD_H
#define GINTEGRALPAYLOAD_H 1

/**
 * \file GIntegralPayload.h
 * \brief Defines the GIntegralPayload structure.
 *
 * GIntegralPayload holds integrated quantities such as crate, slot, channel,
 * charge, and time for a detector electronics module.
 */

#include "glogger.h"
#include <vector>

struct GIntegralPayload {
public:
	/**
	 * \brief Constructs a GIntegralPayload.
	 * \param c Crate number.
	 * \param s Slot number.
	 * \param ch Channel number.
	 * \param q Charge value.
	 * \param t Time value.
	 * \param logger Pointer to a GLogger instance.
	 */
	GIntegralPayload(int c, int s, int ch, int q, int t, std::shared_ptr<GLogger> logger)
		: log(std::move(logger)), crate(c), slot(s), channel(ch), charge(q), time(t) {
		log->debug(CONSTRUCTOR, "GIntegralPayload crate ", crate, " slot ", slot, " channel ", channel, " charge ",
		           charge, " time ", time);
	}

	/**
	 * \brief Destructor for GIntegralPayload.
	 */
	~GIntegralPayload() {
		log->debug(DESTRUCTOR, "GIntegralPayload crate ", crate, " slot ", slot, " channel ", channel, " charge ",
		           charge, " time ", time);
	}

	/**
	 * \brief Returns the payload as a vector of integers.
	 * \return A vector containing crate, slot, channel, charge, and time.
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
	std::shared_ptr<GLogger> log;     ///< Logger instance
	int                      crate;   ///< Crate number.
	int                      slot;    ///< Slot number.
	int                      channel; ///< Channel number.
	int                      charge;  ///< Charge value.
	int                      time;    ///< Time value.
};

#endif
