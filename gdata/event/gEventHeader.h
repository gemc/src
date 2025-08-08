#pragma once

// gemc
#include "gbase.h"

// C++
#include <atomic>
#include <string>


/**
 * \file GEventHeader.h
 * \brief Defines the header for event data collection.
 *
 * This header contains event-related information such as the event number,
 * thread ID, and a timestamp. It is used by the GEventDataCollection class.
 */

constexpr const char* GDATAHEADER_LOGGER = "event_header";

namespace gheader {
inline GOptions defineOptions() {
	auto goptions = GOptions(GDATAHEADER_LOGGER);
	return goptions;
}
}


class GEventHeader : public GBase<GEventHeader> {
public:
	/**
	 * \brief Constructs a GEventDataCollectionHeader.
	 *
	 * The event number is obtained from aEvent->GetEventID(), and the thread ID from G4Threading::G4GetThreadId().
	 *
	 * \param n The local event number.
	 * \param tid The thread ID.
	 * \param gopts Pointer to a GOptions instance.
	 */
	GEventHeader(const std::shared_ptr<GOptions>& gopts, int n, int tid) :
		GBase(gopts, GDATAHEADER_LOGGER),
		g4localEventNumber(n),
		threadID(tid) {
		timeStamp = assignTimeStamp();
		log->debug(CONSTRUCTOR, "GEventDataCollectionHeader");
		log->info(1, "\n",
		          TPOINTITEM, " Event Number:  ", g4localEventNumber, "\n",
		          TPOINTITEM, " Thread ID:  ", threadID, "\n",
		          TPOINTITEM, " Time Stamp:  ", timeStamp);
	}

	/**
	 * \brief Factory method to create a GEventDataCollectionHeader with a unique event number.
	 * \param gopts A shared pointer to Goptions.
	 * \return A unique_ptr to the created GEventDataCollectionHeader.
	 */
	static std::unique_ptr<GEventHeader> create(const std::shared_ptr<GOptions>& gopts, int tid = -1) {
		int eventNumber = globalEventHeaderCounter.fetch_add(1, std::memory_order_relaxed);
		int threadID    = tid; // Example: cycle through 3 thread IDs
		if (threadID < 0) {
			threadID = eventNumber % 8; // default to 8 threads if not provided
		}

		return std::make_unique<GEventHeader>(gopts, eventNumber, threadID);
	}

	/**
	 * \brief Gets the timestamp.
	 * \return The timestamp as a string.
	 */
	[[nodiscard]] inline std::string getTimeStamp() const { return timeStamp; }

	/**
	 * \brief Gets the local event number.
	 * \return The event number.
	 */
	[[nodiscard]] inline int getG4LocalEvn() const { return g4localEventNumber; }

	/**
	 * \brief Gets the thread ID.
	 * \return The thread ID.
	 */
	[[nodiscard]] inline int getThreadID() const { return threadID; }

private:
	int g4localEventNumber; ///< G4Run-local event number.
	int threadID;           ///< Thread ID.

	/**
	 * \brief Assigns a timestamp using the local time.
	 *
	 * Uses C functions to format the time as a string.
	 *
	 * \return A string representing the timestamp.
	 */
	static std::string assignTimeStamp() {
		time_t     now = time(nullptr);
		struct tm* ptm = localtime(&now);
		char       buffer[32];
		// Format: Mo, 15.06.2009 20:20:00
		strftime(buffer, 32, "%a, %m.%d.%Y %H:%M:%S", ptm);
		return {buffer};
	}

	std::string timeStamp; ///< The timestamp

	/// Static thread-safe event counter - used for testing only
	static std::atomic<int> globalEventHeaderCounter;
};
