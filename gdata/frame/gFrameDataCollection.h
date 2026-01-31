#pragma once

/**
 * \file gFrameDataCollection.h
 * \brief Defines \ref GFrameDataCollection a container for frame-level integrated payloads.
 *
 * \details
 * A frame collection groups multiple \ref GIntegralPayload objects under a single
 * \ref GFrameHeader. This models streaming/readout output where many channels may
 * fire within a time window ("frame").
 *
 * Ownership model (current implementation):
 * - \ref GFrameDataCollection owns the \ref GFrameHeader pointer passed at construction
 *   and deletes it in the destructor.
 * - Payloads are allocated with \c new and deleted in the destructor.
 *
 * \note The class uses raw pointers today. If/when modernized, the natural replacement is
 * \c std::unique_ptr<GFrameHeader> and \c std::vector<std::unique_ptr<GIntegralPayload>>.
 *
 * \warning Because raw pointers are used, callers must follow the ownership rules strictly:
 * - do not delete the header after passing it to the constructor
 * - do not delete payload pointers returned by \ref GFrameDataCollection::getIntegralPayload()
 */

#include "gFrameHeader.h"
#include "gIntegralPayload.h"
#include "gdataConventions.h"
#include <vector>

class GFrameDataCollection {
public:
	/**
	 * \brief Construct a frame data collection.
	 *
	 * \details
	 * Ownership:
	 * - \p header is adopted by this object and deleted in the destructor.
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
	 * Deletes:
	 * - the owned frame header
	 * - all owned payload pointers
	 * - the payload vector container
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
	 * The payload is passed as a vector to support a generic "packed" interface,
	 * typically used when data come from external buffers or electronics emulators.
	 *
	 * Expected layout (size must be exactly 5):
	 * - payload[0] = crate
	 * - payload[1] = slot
	 * - payload[2] = channel
	 * - payload[3] = charge
	 * - payload[4] = time
	 *
	 * On success:
	 * - a new \ref GIntegralPayload is allocated and stored internally.
	 *
	 * On failure:
	 * - \ref ERR_WRONGPAYLOAD is reported via the logger.
	 *
	 * \param payload Packed payload vector (must have size 5).
	 */
	void addIntegralPayload(std::vector<int> payload) const {
		if (payload.size() == 5) {
			int crate   = payload[0];
			int slot    = payload[1];
			int channel = payload[2];
			int charge  = payload[3];
			int time    = payload[4];

			auto gpayload = new GIntegralPayload(crate, slot, channel, charge, time, log);
			integralPayloads->push_back(gpayload);
			log->debug(NORMAL, " adding integral payload for crate ", crate, " slot ", slot, " channel ", channel,
			           " charge ", charge, " time ", time);
		}
		else {
			log->error(ERR_WRONGPAYLOAD, "payload size is not 5 but ", payload.size());
		}
	}

	/**
	 * \brief Placeholder for adding event-level information into the frame.
	 *
	 * \details
	 * Intended usage (future):
	 * - accept an event number (or event object)
	 * - integrate it into this frame's payload list (collect channels hit during the frame window)
	 *
	 * \param evn Event number.
	 */
	void addEvent(int evn);

	/**
	 * \brief Placeholder decision hook: should this frame be emitted/written?
	 *
	 * \details
	 * Intended usage (future):
	 * - decide whether enough data has accumulated (or time window has elapsed)
	 * - trigger writing this frame to an output stream
	 *
	 * \return True if frame should be written.
	 */
	[[nodiscard]] bool shouldWriteFrame() const;

	/**
	 * \brief Get the owned frame header (read-only).
	 *
	 * \details
	 * The returned pointer remains valid as long as this \ref GFrameDataCollection exists.
	 *
	 * \return Pointer to the frame header (owned by this object).
	 */
	[[nodiscard]] inline const GFrameHeader* getHeader() const { return gevent_header; }

	/**
	 * \brief Get the stored payload pointers (read-only).
	 *
	 * \warning Pointers are owned by this object and remain valid only as long as the
	 * collection exists. Callers must not delete them.
	 *
	 * \return Pointer to the internal payload vector.
	 */
	[[nodiscard]] inline const std::vector<GIntegralPayload*>* getIntegralPayload() const { return integralPayloads; }

	/**
	 * \brief Convenience getter for frame ID.
	 * \return Frame ID as stored in the header.
	 */
	[[nodiscard]] inline long int getFrameID() const { return gevent_header->getFrameID(); }

private:
	std::shared_ptr<GLogger> log; ///< Logger instance.
	GFrameHeader* gevent_header = nullptr; ///< Owned frame header (deleted in destructor).
	std::vector<GIntegralPayload*>* integralPayloads; ///< Owned payload pointers (deleted in destructor).
};
