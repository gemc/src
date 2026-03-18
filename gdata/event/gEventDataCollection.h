#pragma once

/**
 * \file gEventDataCollection.h
 * \brief Defines GEventDataCollection, the event-level aggregation of detector hit data.
 *
 * \details
 * GEventDataCollection groups all detector-local hit data produced during one event.
 *
 * Primary organization:
 * \code
 * detector name (std::string) -> GDataCollection
 *                                - vector<unique_ptr<GTrueInfoData>>
 *                                - vector<unique_ptr<GDigitizedData>>
 * \endcode
 *
 * Event semantics:
 * - each insertion appends one hit-side object to the target detector entry
 * - detector entries are created lazily on first use
 * - ownership of inserted objects is transferred to this collection
 *
 * Typical workflow:
 * - create one event container
 * - add truth and digitized objects as hits are produced
 * - pass the completed event container downstream for output, analysis, or run integration
 */

#include "gDataCollection.h"
#include "gEventHeader.h"

// C++
#include <map>
#include <string>

constexpr const char* GEVENTDATA_LOGGER = "gevent_data";

namespace gevent_data {

/**
 * \brief Aggregates the option groups needed by event-level data containers.
 *
 * \details
 * The returned bundle includes:
 * - event-header options
 * - true-data options
 * - digitized-data options
 * - touchable-related options used by example identity creation
 *
 * \return Composite options group rooted at \c GEVENTDATA_LOGGER.
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
 * \brief Owns all detector-local data for one event.
 *
 * \details
 * The object combines:
 * - one owned GEventHeader describing the event
 * - one map of detector names to GDataCollection instances
 *
 * Each detector entry can contain:
 * - zero or more truth objects
 * - zero or more digitized objects
 *
 * The class does not enforce structural invariants such as matching truth and digitized counts.
 * Applications that require such guarantees should validate them at a higher level.
 */
class GEventDataCollection : public GBase<GEventDataCollection>
{
public:
	/**
	 * \brief Constructs an event data collection with an owned header.
	 *
	 * \details
	 * Ownership of \p header is transferred to the collection.
	 *
	 * \param gopts  Shared options used to configure logging and related behavior.
	 * \param header Owned event header.
	 */
	GEventDataCollection(const std::shared_ptr<GOptions>& gopts, std::unique_ptr<GEventHeader> header)
		: GBase(gopts, GDATAEVENTHEADER_LOGGER), gevent_header(std::move(header)) {
	}

	/**
	 * \brief Appends one truth object to the specified detector entry.
	 *
	 * \details
	 * If the detector key does not exist yet, a new GDataCollection is created automatically.
	 * Ownership of \p data is transferred to the target detector entry.
	 *
	 * \param sdName Sensitive detector name used as the map key.
	 * \param data   Truth object to store.
	 */
	void addDetectorTrueInfoData(const std::string& sdName, std::unique_ptr<GTrueInfoData> data);

	/**
	 * \brief Appends one digitized object to the specified detector entry.
	 *
	 * \details
	 * If the detector key does not exist yet, a new GDataCollection is created automatically.
	 * Ownership of \p data is transferred to the target detector entry.
	 *
	 * \param sdName Sensitive detector name used as the map key.
	 * \param data   Digitized object to store.
	 */
	void addDetectorDigitizedData(const std::string& sdName, std::unique_ptr<GDigitizedData> data);

	/**
	 * \brief Returns read-only access to the owned event header.
	 *
	 * \return Const reference to the owned event-header pointer.
	 */
	[[nodiscard]] auto getHeader() const -> const std::unique_ptr<GEventHeader>& { return gevent_header; }

	/**
	 * \brief Returns read-only access to the detector map for this event.
	 *
	 * \details
	 * Keys are sensitive detector names and values are per-detector GDataCollection instances.
	 *
	 * \return Const reference to the detector map.
	 */
	[[nodiscard]] auto getDataCollectionMap() const -> const std::map<std::string, std::unique_ptr<GDataCollection>>& {
		return gdataCollectionMap;
	}

	/**
	 * \brief Returns the event number stored in the owned header.
	 *
	 * \details
	 * This is a convenience wrapper around the header accessor.
	 *
	 * \return Event number.
	 */
	[[nodiscard]] auto getEventNumber() const -> int { return gevent_header->getG4LocalEvn(); }

	/**
	 * \brief Creates a minimal example event containing one detector entry and one hit pair.
	 *
	 * \details
	 * This helper is intended for examples and tests.
	 * It creates:
	 * - a fresh GEventHeader
	 * - a new event container
	 * - one digitized object under detector \c ctof
	 * - one truth object under detector \c ctof
	 *
	 * \param gopts Shared options.
	 * \return Shared pointer to the created event collection.
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
	/// Owned event header describing this event.
	std::unique_ptr<GEventHeader> gevent_header;

	/// Per-detector data map keyed by sensitive detector name.
	std::map<std::string, std::unique_ptr<GDataCollection>> gdataCollectionMap;

	/// Static thread-safe counter reserved for tests or future example helpers.
	static std::atomic<int> globalEventDataCollectionCounter;
};