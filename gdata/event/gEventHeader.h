#pragma once

// gemc
#include "gbase.h"

// C++
#include <atomic>
#include <string>

/**
 * \file gEventHeader.h
 * \brief Defines \ref GEventHeader metadata for an event data collection.
 *
 * \details
 * The event header records:
 * - a local event number (\c g4localEventNumber)
 * - a thread identifier (\c threadID)
 * - a timestamp string (\c timeStamp)
 *
 * In production GEMC/Geant4, event number and thread ID would typically come from Geant4
 * (e.g. G4Event and G4Threading). In this library, \ref GEventHeader::create() provides a
 * deterministic generator for examples and tests.
 *
 * \note Time stamp is generated using the local system time at construction.
 */

constexpr const char* GDATAEVENTHEADER_LOGGER = "event_header";

namespace geventheader {
/**
 * \brief Defines \ref GOptions for the event-header logger domain.
 *
 * \details
 * Event header logging can be enabled/controlled by including this in composite option bundles.
 */
inline GOptions defineOptions() {
	auto goptions = GOptions(GDATAEVENTHEADER_LOGGER);
	return goptions;
}
} // namespace geventheader

/**
 * \brief Minimal event metadata header: event number, thread id, and timestamp.
 *
 * \details
 * This object is typically owned by \ref GEventDataCollection as a \c std::unique_ptr.
 *
 * It is primarily used for:
 * - labeling events in logs/output
 * - reproducing the event/thread provenance for debugging
 */
class GEventHeader : public GBase<GEventHeader> {
public:
	/**
	 * \brief Construct an event header with explicit values.
	 *
	 * \details
	 * The constructor:
	 * - assigns \c timeStamp based on local time
	 * - emits an informational log summarizing the header values
	 *
	 * \param gopts Shared options object used to configure logging and behavior.
	 * \param n     Local event number.
	 * \param tid   Thread ID associated with this event.
	 */
	GEventHeader(const std::shared_ptr<GOptions>& gopts, int n, int tid)
		: GBase(gopts, GDATAEVENTHEADER_LOGGER), g4localEventNumber(n), threadID(tid) {
		timeStamp = assignTimeStamp();
		log->debug(CONSTRUCTOR, "GEventHeader");
		log->info(1, "\n",
		          TPOINTITEM, " Event Number:  ", g4localEventNumber, "\n",
		          TPOINTITEM, " Thread ID:  ", threadID, "\n",
		          TPOINTITEM, " Time Stamp:  ", timeStamp);
	}

	/**
	 * \brief Factory method used by examples/tests to create a header with a unique event number.
	 *
	 * \details
	 * If \p tid is negative, a default thread ID is derived from the event number
	 * (currently mod 8) to mimic multi-threaded execution.
	 *
	 * Threading notes:
	 * - Uses an atomic counter so that concurrent calls from multiple threads produce unique event numbers.
	 *
	 * \param gopts Shared options.
	 * \param tid   Optional thread ID override.
	 * \return Newly created event header.
	 */
	static std::unique_ptr<GEventHeader> create(const std::shared_ptr<GOptions>& gopts, int tid = -1) {
		int eventNumber = globalEventHeaderCounter.fetch_add(1, std::memory_order_relaxed);
		int threadID_   = tid;
		if (threadID_ < 0) {
			threadID_ = eventNumber % 8; // default to 8 threads if not provided
		}
		return std::make_unique<GEventHeader>(gopts, eventNumber, threadID_);
	}

	/**
	 * \brief Get the formatted timestamp string.
	 * \return Timestamp string.
	 */
	[[nodiscard]] inline std::string getTimeStamp() const { return timeStamp; }

	/**
	 * \brief Get the local event number.
	 * \details This is "run-local" in typical Geant4 usage.
	 * \return Event number.
	 */
	[[nodiscard]] inline int getG4LocalEvn() const { return g4localEventNumber; }

	/**
	 * \brief Get the thread ID associated with this event.
	 * \return Thread ID.
	 */
	[[nodiscard]] inline int getThreadID() const { return threadID; }

private:
	int g4localEventNumber; ///< Event number (run-local in typical Geant4 usage).
	int threadID;           ///< Thread ID (diagnostic/labeling).

	/**
	 * \brief Create a timestamp string using local time.
	 *
	 * \details
	 * Format:
	 * \code
	 *   "Mon 01.30.2026 15:04:05"
	 * \endcode
	 * (weekday mm.dd.yyyy hh:mm:ss).
	 *
	 * \return Timestamp string.
	 */
	static std::string assignTimeStamp() {
		time_t     now = time(nullptr);
		struct tm* ptm = localtime(&now);
		char       buffer[32];
		strftime(buffer, 32, "%a %m.%d.%Y %H:%M:%S", ptm);
		return {buffer};
	}

	std::string timeStamp; ///< Timestamp string.

	/// Static thread-safe event counter - used for testing/examples only.
	static std::atomic<int> globalEventHeaderCounter;
};
