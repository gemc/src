#pragma once

// gemc
#include "gbase.h"

// C++
#include <atomic>
#include <string>

/**
 * \file gEventHeader.h
 * \brief Defines GEventHeader, the metadata header associated with one event container.
 *
 * \details
 * GEventHeader stores the minimal identifying metadata for one event:
 * - a local event number
 * - a thread identifier
 * - a formatted timestamp string
 *
 * Typical usage:
 * - construct explicitly when event identity is already known
 * - use \ref GEventHeader::create "create()" in examples and tests to generate deterministic headers
 *
 * The header is normally owned by GEventDataCollection.
 */

constexpr const char* GDATAEVENTHEADER_LOGGER = "event_header";

namespace geventheader {

/**
 * \brief Defines the options subtree used by the event-header logger domain.
 *
 * \details
 * Higher-level modules can aggregate this option group into their own configuration bundles
 * so that event-header verbosity can be controlled centrally.
 *
 * \return Options group rooted at \c GDATAEVENTHEADER_LOGGER.
 */
inline GOptions defineOptions() {
	auto goptions = GOptions(GDATAEVENTHEADER_LOGGER);
	return goptions;
}

} // namespace geventheader

/**
 * \brief Stores minimal metadata for one event.
 *
 * \details
 * The object provides a compact event label and provenance bundle containing:
 * - the local event number
 * - the thread identifier used for diagnostics
 * - a construction-time timestamp string
 *
 * Ownership:
 * - this object is typically owned exclusively by GEventDataCollection
 */
class GEventHeader : public GBase<GEventHeader>
{
public:
	/**
	 * \brief Constructs an event header with explicit event and thread identifiers.
	 *
	 * \details
	 * The constructor also assigns a local-time timestamp string and emits an informational
	 * summary through the logger.
	 *
	 * \param gopts Shared options used to configure logging and related behavior.
	 * \param n     Local event number.
	 * \param tid   Thread identifier associated with the event.
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
	 * \brief Creates a header with a unique event number for tests and examples.
	 *
	 * \details
	 * If \p tid is negative, a synthetic thread identifier is derived from the event number.
	 * The generated event number comes from a static atomic counter so concurrent example code
	 * can obtain unique values.
	 *
	 * \param gopts Shared options.
	 * \param tid   Optional thread identifier override.
	 * \return Newly created event header.
	 */
	static std::unique_ptr<GEventHeader> create(const std::shared_ptr<GOptions>& gopts, int tid = -1) {
		int eventNumber = globalEventHeaderCounter.fetch_add(1, std::memory_order_relaxed);
		int threadID_   = tid;
		if (threadID_ < 0) {
			threadID_ = eventNumber % 8;
		}
		return std::make_unique<GEventHeader>(gopts, eventNumber, threadID_);
	}

	/**
	 * \brief Returns the formatted timestamp string.
	 *
	 * \return Timestamp assigned at construction.
	 */
	[[nodiscard]] inline std::string getTimeStamp() const { return timeStamp; }

	/**
	 * \brief Returns the local event number.
	 *
	 * \details
	 * In typical Geant4-style usage, this value is run-local.
	 *
	 * \return Local event number.
	 */
	[[nodiscard]] inline int getG4LocalEvn() const { return g4localEventNumber; }

	/**
	 * \brief Returns the thread identifier associated with the event.
	 *
	 * \return Thread identifier.
	 */
	[[nodiscard]] inline int getThreadID() const { return threadID; }

private:
	/// Event number local to the current run or example sequence.
	int g4localEventNumber;

	/// Thread identifier used for diagnostics and provenance labeling.
	int threadID;

	/**
	 * \brief Builds the formatted timestamp string using local time.
	 *
	 * \details
	 * Format:
	 * \code
	 * Mon 01.30.2026 15:04:05
	 * \endcode
	 *
	 * \return Formatted timestamp string.
	 */
	static std::string assignTimeStamp() {
		time_t     now = time(nullptr);
		struct tm* ptm = localtime(&now);
		char       buffer[32];
		strftime(buffer, 32, "%a %m.%d.%Y %H:%M:%S", ptm);
		return {buffer};
	}

	/// Timestamp string assigned at construction.
	std::string timeStamp;

	/// Static thread-safe event counter used only by \ref GEventHeader::create "create()".
	static std::atomic<int> globalEventHeaderCounter;
};