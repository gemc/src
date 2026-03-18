#pragma once

/**
 * \file gFrameDataCollection.h
 * \brief Defines GFrameDataCollection, a container for frame-level integrated payloads.
 *
 * \details
 * A frame collection groups multiple GIntegralPayload objects under a single GFrameHeader.
 * This models streaming/readout output where many channels may fire within a time window.
 *
 * Ownership model (current implementation):
 * - GFrameDataCollection adopts the GFrameHeader pointer passed at construction and deletes it
 *   in the destructor
 * - payloads are allocated with \c new in \ref GFrameDataCollection::addIntegralPayload "addIntegralPayload()"
 *   and deleted in the destructor
 * - the internal payload container is also heap-allocated and deleted in the destructor
 *
 * \note
 * The class uses raw pointers today. If it is modernized in the future, the natural replacement is
 * \c std::unique_ptr<GFrameHeader> together with
 * \c std::vector<std::unique_ptr<GIntegralPayload>>.
 *
 * \warning
 * Because raw pointers are used, callers must follow the ownership rules strictly:
 * - do not delete the header after passing it to the constructor
 * - do not delete payload pointers returned by \ref GFrameDataCollection::getIntegralPayload "getIntegralPayload()"
 * - do not store returned pointers beyond the lifetime of the owning GFrameDataCollection
 */

#include "gFrameHeader.h"
#include "gIntegralPayload.h"
#include "gdataConventions.h"
#include <vector>

/**
 * \defgroup gdata_frame_collection GData frame collection
 * \brief Frame-level container owning one frame header and many integral payloads.
 *
 * \details
 * This topic documents the frame aggregation object used for streaming-style output. It groups
 * payload samples under one frame header and provides the packed-payload insertion interface.
 */

/**
 * \brief Owns one frame header and the list of payloads accumulated for that frame.
 * \ingroup gdata_frame_collection
 */
class GFrameDataCollection
{
public:
	/**
	 * \brief Construct a frame data collection.
	 *
	 * \details
	 * Ownership:
	 * - \p header is adopted by this object and deleted in the destructor
	 *
	 * Internal storage:
	 * - the payload pointer vector is allocated on the heap for historical reasons
	 *
	 * \param header Frame header pointer. Ownership is transferred to this object.
	 * \param logger Logger instance used for diagnostics.
	 */
	GFrameDataCollection(GFrameHeader* header, std::shared_ptr<GLogger> logger)
		: log(logger), gevent_header(header) {
		log->debug(CONSTRUCTOR, "GFrameDataCollection");
		integralPayloads = new std::vector<GIntegralPayload*>();
	}

	/**
	 * \brief Destructor.
	 *
	 * \details
	 * Deletes in the following order:
	 * - the owned frame header
	 * - all owned payload pointers
	 * - the payload vector container
	 *
	 * \warning
	 * Any pointers previously returned by \ref GFrameDataCollection::getHeader "getHeader()"
	 * or \ref GFrameDataCollection::getIntegralPayload "getIntegralPayload()"
	 * become invalid after destruction.
	 */
	~GFrameDataCollection() {
		log->debug(DESTRUCTOR, "GFrameDataCollection");
		delete gevent_header;
		for (auto* payload : *integralPayloads) { delete payload; }
		delete integralPayloads;
	}

public:
	/**
	 * \brief Add one integral payload to this frame.
	 *
	 * \details
	 * The payload is passed as a vector to support a generic packed interface, typically used when
	 * data come from external buffers or electronics emulators.
	 *
	 * Expected layout, size must be exactly 5:
	 * - payload[0] = crate
	 * - payload[1] = slot
	 * - payload[2] = channel
	 * - payload[3] = charge
	 * - payload[4] = time
	 *
	 * On success:
	 * - a new GIntegralPayload is allocated and stored internally
	 *
	 * On failure:
	 * - \ref ERR_WRONGPAYLOAD is reported via the logger
	 * - no payload is added
	 *
	 * \param payload Packed payload vector with exactly five integer entries.
	 */
	void addIntegralPayload(std::vector<int> payload) const {
		// Decode the fixed packed layout only when the payload size matches the contract.
		if (payload.size() == 5) {
			int crate   = payload[0];
			int slot    = payload[1];
			int channel = payload[2];
			int charge  = payload[3];
			int time    = payload[4];

			// Create and store one owned payload object for this frame.
			auto gpayload = new GIntegralPayload(crate, slot, channel, charge, time, log);
			integralPayloads->push_back(gpayload);
			log->debug(NORMAL, " adding integral payload for crate ", crate, " slot ", slot, " channel ", channel,
			           " charge ", charge, " time ", time);
		}
		else {
			// Reject malformed packed payloads and keep the collection unchanged.
			log->error(ERR_WRONGPAYLOAD, "payload size is not 5 but ", payload.size());
		}
	}

	/**
	 * \brief Placeholder for adding event-level information into the frame.
	 *
	 * \details
	 * Intended future usage:
	 * - accept an event number or event object
	 * - integrate it into this frame's payload list by collecting channels hit during the frame window
	 *
	 * \param evn Event number.
	 */
	void addEvent(int evn);

	/**
	 * \brief Placeholder decision hook indicating whether this frame should be emitted.
	 *
	 * \details
	 * Intended future usage:
	 * - decide whether enough data has accumulated
	 * - decide whether the frame time window has elapsed
	 * - trigger writing this frame to an output stream
	 *
	 * \return \c true if the frame should be written.
	 */
	[[nodiscard]] bool shouldWriteFrame() const;

	/**
	 * \brief Returns the owned frame header.
	 *
	 * \details
	 * The returned pointer remains valid as long as this GFrameDataCollection exists.
	 * The header is owned by this object and must not be deleted by the caller.
	 *
	 * \return Pointer to the frame header owned by this object.
	 */
	[[nodiscard]] inline const GFrameHeader* getHeader() const { return gevent_header; }

	/**
	 * \brief Returns the stored payload pointers.
	 *
	 * \details
	 * The returned pointers:
	 * - are owned by this object
	 * - remain valid only as long as the collection exists
	 * - must not be deleted by the caller
	 *
	 * \return Pointer to the internal payload vector.
	 */
	[[nodiscard]] inline const std::vector<GIntegralPayload*>* getIntegralPayload() const { return integralPayloads; }

	/**
	 * \brief Convenience getter for the frame ID.
	 *
	 * \details
	 * Equivalent to calling \ref GFrameHeader::getFrameID "getFrameID()" on the owned header.
	 *
	 * \return Frame ID as stored in the header.
	 */
	[[nodiscard]] inline long int getFrameID() const { return gevent_header->getFrameID(); }

private:
	std::shared_ptr<GLogger>        log;                     ///< Logger instance.
	GFrameHeader*                   gevent_header = nullptr; ///< Owned frame header deleted in the destructor.
	std::vector<GIntegralPayload*>* integralPayloads;        ///< Owned payload pointers deleted in the destructor.
};