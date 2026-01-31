#pragma once

/**
 * \file gRunDataCollection.h
 * \brief Defines \ref GRunDataCollection run-level aggregation of detector data.
 *
 * \details
 * A run collection integrates event-level data into a run summary.
 *
 * It owns:
 * - a \ref GRunHeader describing run ID and integrated event count
 * - a map from sensitive detector name to \ref GDataCollection (the per-detector accumulator)
 *
 * Integration is performed by consuming \ref GEventDataCollection objects and:
 * - iterating over each detector’s per-hit data vectors
 * - accumulating observables into a per-detector \ref GDataCollection entry
 *
 * The resulting structure typically has:
 * - one \ref GDataCollection per detector
 * - one integrated \ref GTrueInfoData and one integrated \ref GDigitizedData entry per detector
 *   (vector size 1) depending on how \ref GDataCollection integration is used.
 *
 * \note This class currently does not increment \ref GRunHeader::events_processed.
 *       If you want that counter to reflect integrated events, the caller (or this class)
 *       must call \ref GRunHeader::increment_events_processed() once per integrated event.
 */

#include "gRunHeader.h"
#include "gEventDataCollection.h"

// C++
#include <map>
#include <string>

constexpr const char* GRUNDATA_LOGGER = "grun_data";

namespace grun_data {
/**
 * \brief Aggregated options for run-level data collection.
 *
 * \details
 * Combines options from:
 * - event header + event data collection
 * - run header + run data collection
 * - true/digitized data
 * - touchable (for hit identity creation in examples)
 *
 * This is intended to provide a single "options bundle" for examples and applications.
 */
inline auto defineOptions() -> GOptions {
	auto goptions = GOptions(GRUNDATA_LOGGER);
	goptions      += geventheader::defineOptions();
	goptions      += gevent_data::defineOptions();
	goptions      += grun_header::defineOptions();
	goptions      += gtrue_data::defineOptions();
	goptions      += gdigi_data::defineOptions();
	goptions      += gtouchable::defineOptions();
	return goptions;
}
} // namespace grun_data

/**
 * \brief Run-level container that integrates per-event detector data into per-detector summaries.
 *
 * \details
 * The main API is \ref collect_event_data_collection(), which:
 * - loops over detectors present in the event
 * - loops over all hits for each detector (truth and digitized)
 * - delegates integration to \ref GDataCollection accumulation methods
 *
 * The per-detector map is keyed by sensitive detector name.
 */
class GRunDataCollection : public GBase<GRunDataCollection>
{
public:
	/**
	 * \brief Construct a run data collection.
	 *
	 * \details
	 * The header stores metadata such as run ID and (optionally) number of integrated events.
	 *
	 * \param gopts  Shared options object used to configure logging and behavior.
	 * \param header Owned run header describing this run.
	 */
	GRunDataCollection(const std::shared_ptr<GOptions>& gopts, std::unique_ptr<GRunHeader> header)
		: GBase(gopts, GRUNDATA_LOGGER), grun_header(std::move(header)) {
	}

	/**
	 * \brief Integrate one event data collection into this run summary.
	 *
	 * \details
	 * For each detector present in \p edc, integrates all per-hit entries:
	 * - truth hits (via \ref collectDetectorTrueInfoData)
	 * - digitized hits (via \ref collectDetectorDigitizedData)
	 *
	 * The integration semantics for individual hits are implemented in \ref GDataCollection:
	 * - first hit creates the integrated entry
	 * - subsequent hits contribute by summation of scalars
	 *
	 * \param edc Event-level container to integrate.
	 */
	void collect_event_data_collection(const std::shared_ptr<GEventDataCollection> edc);

	/**
	 * \brief Access the owned run header.
	 * \return Const reference to the header unique_ptr.
	 */
	[[nodiscard]] auto getHeader() const -> const std::unique_ptr<GRunHeader>& { return grun_header; }

	/**
	 * \brief Access the per-detector run summary map.
	 *
	 * \details
	 * Key: sensitive detector name.
	 * Value: per-detector \ref GDataCollection accumulator.
	 *
	 * \return Const reference to the detector map.
	 */
	[[nodiscard]] auto getDataCollectionMap() const -> const std::map<std::string, std::unique_ptr<GDataCollection>>& {
		return gdataCollectionMap;
	}

	/**
	 * \brief Convenience accessor for the run number.
	 * \return Run ID stored in the header.
	 */
	[[nodiscard]] auto getRunNumber() const -> int { return grun_header->getRunID(); }

	/**
	 * \brief Number of events integrated into this run summary.
	 *
	 * \details
	 * This value is stored in \ref GRunHeader and incremented by
	 * \ref GRunHeader::increment_events_processed().
	 *
	 * \return Count stored in the header.
	 */
	[[nodiscard]] auto get_events_processed() const -> int { return grun_header->get_events_processed(); }

private:
	std::unique_ptr<GRunHeader> grun_header; ///< Owned run header.

	/// Per-detector accumulated data keyed by sensitive detector name.
	std::map<std::string, std::unique_ptr<GDataCollection>> gdataCollectionMap;

	/**
	 * \brief Integrate one true-hit entry into the detector accumulator.
	 *
	 * \details
	 * Ensures the detector entry exists in \ref gdataCollectionMap and then delegates
	 * integration to \ref GDataCollection::collectTrueInfosData().
	 *
	 * \param sdName Sensitive detector name.
	 * \param data   True-hit object to integrate (not owned; deep-copied internally).
	 */
	void collectDetectorTrueInfoData(const std::string& sdName, const std::unique_ptr<GTrueInfoData>& data);

	/**
	 * \brief Integrate one digitized-hit entry into the detector accumulator.
	 *
	 * \details
	 * Ensures the detector entry exists and delegates integration to
	 * \ref GDataCollection::collectDigitizedData().
	 *
	 * \param sdName Sensitive detector name.
	 * \param data   Digitized-hit object to integrate (not owned; deep-copied internally).
	 */
	void collectDetectorDigitizedData(const std::string& sdName, const std::unique_ptr<GDigitizedData>& data);
};
