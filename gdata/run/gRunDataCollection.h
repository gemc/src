#pragma once

/**
 * \file gRunDataCollection.h
 * \brief Defines GRunDataCollection, the run-level aggregation of detector data.
 *
 * \details
 * GRunDataCollection integrates event-level content into a run summary.
 *
 * It owns:
 * - one GRunHeader with run metadata and counters
 * - one map from sensitive detector name to GDataCollection
 *
 * Current implementation status:
 * - event integration currently processes digitized detector data
 * - truth-side aggregation is not currently performed by
 *   \ref GRunDataCollection::collect_event_data_collection "collect_event_data_collection()"
 *
 * Resulting structure:
 * - one detector entry per sensitive detector that contributed data
 * - one integrated digitized entry per detector in the usual integrated workflow
 */

#include "gEventDataCollection.h"
#include "gRunHeader.h"

// C++
#include <map>
#include <string>

constexpr const char* GRUNDATA_LOGGER = "grun_data";

namespace grun_data {

/**
 * \brief Aggregates the option groups needed by run-level data containers.
 *
 * \details
 * The returned bundle includes:
 * - event-header options
 * - event-data options
 * - run-header options
 * - digitized-data options
 * - touchable-related options used by examples
 *
 * \return Composite options group rooted at \c GRUNDATA_LOGGER.
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
 * \defgroup gdata_run_collection GData run collection
 * \brief Run-level detector summary built by integrating many event collections.
 *
 * \details
 * This topic documents the object responsible for collecting detector data across events. It owns
 * the run header and the per-detector summary map and also supports merging partial run accumulators.
 */

/**
 * \brief Owns and updates the run-level detector summary map.
 * \ingroup gdata_run_collection
 *
 * \details
 * The object integrates event content detector by detector.
 *
 * Current event-integration behavior:
 * - loops over detector entries in the event
 * - iterates over the digitized hit vector for each detector
 * - accumulates digitized non-SRO scalar observables through GDataCollection
 *
 * This class also supports merging partial run accumulators through
 * \ref GRunDataCollection::merge "merge()".
 */
class GRunDataCollection : public GBase<GRunDataCollection>
{
public:
	/**
	 * \brief Constructs a run data collection with an owned run header.
	 *
	 * \details
	 * Ownership of \p header is transferred to this object.
	 *
	 * \param gopts  Shared options used to configure logging and related behavior.
	 * \param header Owned run header.
	 */
	GRunDataCollection(const std::shared_ptr<GOptions>& gopts, std::unique_ptr<GRunHeader> header)
		: GBase(gopts, GRUNDATA_LOGGER), grun_header(std::move(header)) {
	}

	/**
	 * \brief Integrates one event collection into the run summary.
	 *
	 * \details
	 * Current implementation:
	 * - increments the processed-event counter once
	 * - loops over detector entries present in \p edc
	 * - integrates the digitized hit objects stored under each detector
	 *
	 * The detector-local accumulation rules are implemented in GDataCollection :
	 * - the first integrated entry creates the detector accumulator
	 * - subsequent entries contribute by summation of the supported observables
	 *
	 * \param edc Event collection to integrate.
	 */
	void collect_event_data_collection(const std::shared_ptr<GEventDataCollection> edc);

	/**
	 * \brief Integrates one digitized object into the named detector entry.
	 *
	 * \details
	 * This helper accepts ownership of \p ddata and forwards the content into the detector-local
	 * accumulator path.
	 *
	 * \param sdName Sensitive detector name.
	 * \param ddata  Digitized object to integrate.
	 */
	void collect_event_data_collections(const std::string& sdName, std::unique_ptr<GDigitizedData> ddata);

	/**
	 * \brief Returns read-only access to the owned run header.
	 *
	 * \return Const reference to the owned run-header pointer.
	 */
	[[nodiscard]] auto getHeader() const -> const std::unique_ptr<GRunHeader>& { return grun_header; }

	/**
	 * \brief Returns read-only access to the detector summary map.
	 *
	 * \details
	 * Keys are sensitive detector names and values are per-detector GDataCollection accumulators.
	 *
	 * \return Const reference to the detector map.
	 */
	[[nodiscard]] auto getDataCollectionMap() const -> const std::map<std::string, std::unique_ptr<GDataCollection>>& {
		return gdataCollectionMap;
	}

	/**
	 * \brief Returns the run number stored in the owned header.
	 *
	 * \return Run identifier.
	 */
	[[nodiscard]] auto getRunNumber() const -> int { return grun_header->getRunID(); }

	/**
	 * \brief Returns the total processed-event count stored in the header.
	 *
	 * \return Number of processed events.
	 */
	[[nodiscard]] auto get_events_processed() const -> int { return grun_header->get_events_processed(); }

	/**
	 * \brief Returns the number of processed events that contributed payload.
	 *
	 * \return Number of payload-producing events.
	 */
	[[nodiscard]] auto get_events_with_payload() const -> int { return grun_header->get_events_with_payload(); }

	/**
	 * \brief Merges another run accumulator into this one.
	 *
	 * \details
	 * This method is intended for end-of-run reduction when multiple partial accumulators
	 * have been produced independently.
	 *
	 * Current merge behavior:
	 * - adds the event counters from \p other
	 * - merges the integrated digitized detector data from \p other into this object
	 *
	 * \param other Source run accumulator whose content will be added into this object.
	 */
	void merge(const GRunDataCollection& other);

private:
	/// Owned run header describing this run summary.
	std::unique_ptr<GRunHeader> grun_header;

	/// Per-detector accumulated data keyed by sensitive detector name.
	std::map<std::string, std::unique_ptr<GDataCollection>> gdataCollectionMap;

	/**
	 * \brief Integrates one digitized object into the named detector accumulator.
	 *
	 * \details
	 * If the detector entry does not exist, it is created automatically.
	 * The detector-local accumulation logic itself is delegated to GDataCollection.
	 *
	 * \param sdName Sensitive detector name.
	 * \param data   Digitized object whose content is copied or accumulated.
	 */
	void collectDetectorDigitizedData(const std::string& sdName, const std::unique_ptr<GDigitizedData>& data);
};