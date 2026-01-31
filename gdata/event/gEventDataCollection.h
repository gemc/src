#pragma once

/**
 * \file gEventDataCollection.h
 * \brief Defines \ref GEventDataCollection event-level aggregation of per-detector hit data.
 *
 * \details
 * An event data collection groups all hit data produced during a single event.
 * The primary organization is:
 *
 * \code
 *   detector name (std::string)  ->  GDataCollection
 *                                    - vector<unique_ptr<GTrueInfoData>>
 *                                    - vector<unique_ptr<GDigitizedData>>
 * \endcode
 *
 * ## Event-level semantics
 * - each call to \ref GEventDataCollection::addDetectorTrueInfoData() or \ref GEventDataCollection::addDetectorDigitizedData()
 *   appends one hit entry to the specified detector’s vectors.
 * - the detector entry is created on demand if it does not already exist.
 *
 * ## Ownership
 * - The event collection owns all hit entries via \c std::unique_ptr.
 * - The caller transfers ownership when adding data (via \c std::move).
 *
 * ## Intended usage
 * A typical event loop will:
 * - create one \ref GEventDataCollection per event
 * - for each hit produced by simulation/digitization, push truth/digitized records into the event container
 * - pass the finished event container downstream (output, run integration, analysis, etc.)
 */

#include "gEventHeader.h"
#include "gDataCollection.h"

// C++
#include <map>
#include <string>

constexpr const char* GEVENTDATA_LOGGER = "gevent_data";

namespace gevent_data {
/**
 * \brief Aggregated options for event-level data collection.
 *
 * \details
 * Combines options from:
 * - event header
 * - true/digitized data
 * - touchable (for identity creation in examples)
 *
 * This provides a single "options bundle" for event-level examples/applications.
 */
inline auto defineOptions() -> GOptions {
	auto goptions = GOptions(GEVENTDATA_LOGGER);
	goptions      += geventheader::defineOptions();
	goptions      += gtrue_data::defineOptions();
	goptions      += gdigi_data::defineOptions();
	goptions      += gtouchable::defineOptions();
	return goptions;
}
} // namespace gevent_data

/**
 * \brief Event container that owns per-detector hit data for one event.
 *
 * \details
 * The container is built around a map from sensitive detector name to \ref GDataCollection.
 * Each detector collection stores per-hit truth and digitized objects.
 *
 * The header (\ref GEventHeader) stores identifying metadata such as:
 * - local event number
 * - thread ID label
 * - timestamp string
 */
class GEventDataCollection : public GBase<GEventDataCollection>
{
public:
	/**
	 * \brief Construct an event data collection with an owned header.
	 *
	 * \details
	 * Ownership:
	 * - \p header is moved into this object and is owned exclusively.
	 *
	 * \param gopts   Shared options object used to configure logging and behavior.
	 * \param header  Owned event header.
	 */
	GEventDataCollection(const std::shared_ptr<GOptions>& gopts, std::unique_ptr<GEventHeader> header)
		: GBase(gopts, GDATAEVENTHEADER_LOGGER), gevent_header(std::move(header)) {
	}

	/**
	 * \brief Append one true-hit entry to the specified detector.
	 *
	 * \details
	 * - If \p sdName is new, a per-detector \ref GDataCollection is created automatically.
	 * - Ownership of \p data is transferred to this event container.
	 *
	 * \param sdName Sensitive detector name (map key).
	 * \param data   True-hit object; ownership is transferred to this collection.
	 */
	void addDetectorTrueInfoData(const std::string& sdName, std::unique_ptr<GTrueInfoData> data);

	/**
	 * \brief Append one digitized-hit entry to the specified detector.
	 *
	 * \details
	 * - If \p sdName is new, a per-detector \ref GDataCollection is created automatically.
	 * - Ownership of \p data is transferred to this event container.
	 *
	 * \param sdName Sensitive detector name (map key).
	 * \param data   Digitized-hit object; ownership is transferred to this collection.
	 */
	void addDetectorDigitizedData(const std::string& sdName, std::unique_ptr<GDigitizedData> data);

	/**
	 * \brief Access the owned event header.
	 * \return Const reference to the header unique_ptr.
	 */
	[[nodiscard]] auto getHeader() const -> const std::unique_ptr<GEventHeader>& { return gevent_header; }

	/**
	 * \brief Access the per-detector map for this event.
	 *
	 * \details
	 * Key: sensitive detector name.
	 * Value: per-detector \ref GDataCollection containing per-hit entries.
	 *
	 * \return Const reference to the detector map.
	 */
	[[nodiscard]] auto getDataCollectionMap() const -> const std::map<std::string, std::unique_ptr<GDataCollection>>& {
		return gdataCollectionMap;
	}

	/**
	 * \brief Convenience accessor for the event number.
	 * \return Event number stored in the header.
	 */
	[[nodiscard]] auto getEventNumber() const -> int { return gevent_header->getG4LocalEvn(); }

	/**
	 * \brief Test/example factory: create an event collection with one dummy hit for "ctof".
	 *
	 * \details
	 * This method exists to support examples/tests. It:
	 * - creates a new \ref GEventHeader
	 * - constructs an event data collection
	 * - inserts one \ref GDigitizedData and one \ref GTrueInfoData entry under detector "ctof"
	 *
	 * This provides a minimal "event contains something" baseline for examples.
	 *
	 * \param gopts Shared options.
	 * \return Shared pointer to the created event data collection.
	 */
	static auto create(const std::shared_ptr<GOptions>& gopts) -> std::shared_ptr<GEventDataCollection> {
		auto header = GEventHeader::create(gopts);
		auto edc    = std::make_shared<GEventDataCollection>(gopts, std::move(header));

		auto digi_data = GDigitizedData::create(gopts);
		auto true_data = GTrueInfoData::create(gopts);

		edc->addDetectorDigitizedData("ctof", std::move(digi_data));
		edc->addDetectorTrueInfoData("ctof", std::move(true_data));

		return edc;
	}

private:
	std::unique_ptr<GEventHeader> gevent_header; ///< Owned event header.

	/// Per-detector data map keyed by sensitive detector name.
	std::map<std::string, std::unique_ptr<GDataCollection>> gdataCollectionMap;

	/// Static thread-safe event counter - used for testing/examples only.
	static std::atomic<int> globalEventDataCollectionCounter;
};
