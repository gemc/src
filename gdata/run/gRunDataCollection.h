#pragma once

/**
 * \file gRunDataCollection.h
 * \brief Defines \ref GRunDataCollection, run-level aggregation of detector data.
 *
 * \details
 * A run collection integrates event-level data into a run summary.
 *
 * It owns:
 * - a \ref GRunHeader describing run ID and event counters
 * - a map from sensitive detector name to \ref GDataCollection (the per-detector accumulator)
 *
 * Integration is performed by consuming \ref GEventDataCollection objects and:
 * - iterating over each detector’s per-hit data vectors
 * - accumulating observables into a per-detector \ref GDataCollection entry
 *
 * The resulting structure typically has:
 * - one \ref GDataCollection per detector
 * - one integrated \ref GDigitizedData entry per detector
 *   (vector size 1), depending on how \ref GDataCollection integration is used.
 *
 * \note Event counters
 * This class stores event counters in \ref GRunHeader:
 * - \c events_processed tracks total processed events
 * - \c events_with_payload tracks events that contributed at least one run-mode payload
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
	 * - touchable (for hit identity creation in examples)
	 *
	 * This is intended to provide a single "options bundle" for examples and applications.
	 *
	 * \return Composite options group rooted at \ref GRUNDATA_LOGGER.
	 */
	inline auto defineOptions() -> GOptions {
		auto goptions = GOptions(GRUNDATA_LOGGER);
		goptions      += geventheader::defineOptions();
		goptions      += gevent_data::defineOptions();
		goptions      += grun_header::defineOptions();
		goptions      += gdigi_data::defineOptions();
		goptions      += gtouchable::defineOptions();
		return goptions;
	}
} // namespace grun_data

/**
 * \brief Run-level container that integrates per-event detector data into per-detector summaries.
 *
 * \details
 * The main API is \ref GRunDataCollection::collect_event_data_collection "collect_event_data_collection()",
 * which:
 * - loops over detectors present in the event
 * - loops over all hits for each detector (only the digitized)
 * - delegates integration to \ref GDataCollection accumulation methods
 *
 * The per-detector map is keyed by sensitive detector name.
 *
 * \note Threading and merging
 * This class acts as a single accumulator. If you create one accumulator per thread,
 * you need a higher-level merge strategy (not implemented here).
 */
class GRunDataCollection : public GBase<GRunDataCollection>
{
public:
	/**
	 * \brief Construct a run data collection.
	 *
	 * \details
	 * The header stores metadata such as run ID and event counters.
	 *
	 * Ownership:
	 * - \p header is moved into this object and owned exclusively.
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
	 * - digitized hits (via the private method  \c "collectDetectorDigitizedData()")
	 *
	 * The integration semantics for individual hits are implemented in \ref GDataCollection :
	 * - first hit creates the integrated entry
	 * - subsequent hits contribute by summation of scalars
	 *
	 * This method increments \c events_processed once for the integrated event.
	 *
	 * \param edc Event-level container to integrate.
	 */
	void collect_event_data_collection(const std::shared_ptr<GEventDataCollection> edc);
	void collect_event_data_collections(const std::string&              sdName,
										std::unique_ptr<GDigitizedData> ddata);

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
	 * \brief Total number of processed events associated with this run summary.
	 *
	 * \details
	 * This value is stored in \ref GRunHeader and incremented by
	 * \ref GRunHeader::increment_events_processed "increment_events_processed()".
	 *
	 * \return Count stored in the header.
	 */
	[[nodiscard]] auto get_events_processed() const -> int { return grun_header->get_events_processed(); }

	/**
	 * \brief Number of processed events that contributed run-mode payload.
	 *
	 * \details
	 * This value is stored in \ref GRunHeader and incremented by
	 * \ref GRunHeader::increment_events_with_payload "increment_events_with_payload()".
	 *
	 * \return Count stored in the header.
	 */
	[[nodiscard]] auto get_events_with_payload() const -> int { return grun_header->get_events_with_payload(); }

	/**
	 * \brief Merge another run-level accumulator into this one.
	 *
	 * \details
	 * This adds the already-integrated detector totals from \p other into this object.
	 * It is intended for multithreaded end-of-run reduction, where each worker thread
	 * has produced its own partial \ref GRunDataCollection.
	 *
	 * Detector entries are merged by detector name. For each detector:
	 * - digitized integrated data are accumulated into this object
	 *
	 * Event counters are also merged:
	 * - total processed events
	 * - events with payload
	 *
	 * \param other Source run accumulator to merge into this object.
	 */
	void merge(const GRunDataCollection& other);

private:
	std::unique_ptr<GRunHeader> grun_header; ///< Owned run header.

	/// Per-detector accumulated data keyed by sensitive detector name.
	std::map<std::string, std::unique_ptr<GDataCollection>> gdataCollectionMap;

	/**
	 * \brief Integrate one digitized-hit entry into the detector accumulator.
	 *
	 * \details
	 * Ensures the detector entry exists and delegates integration to
	 * \ref GDataCollection::collectDigitizedData "collectDigitizedData()".
	 *
	 * \param sdName Sensitive detector name.
	 * \param data   Digitized-hit object to integrate (not owned; deep-copied internally).
	 */
	void collectDetectorDigitizedData(const std::string& sdName, const std::unique_ptr<GDigitizedData>& data);
};