#pragma once

/**
 * \file GDataCollection.h
 * \brief Defines the \ref GDataCollection class for storing hit data for a detector.
 *
 * \details
 * A \ref GDataCollection is the per-sensitive-detector container used by both:
 * - \ref GEventDataCollection (event-level): stores many hits (one entry per hit)
 * - \ref GRunDataCollection   (run-level): integrates hits/events into a single entry
 *
 * It holds two independent collections:
 * - \ref GTrueInfoData   "truth" observables derived from simulation / Geant4 tracking.
 * - \ref GDigitizedData  "digitized" observables produced by electronics/digitization logic.
 *
 * ## Ownership model
 * - **Event-level**:
 *   - \ref GDataCollection::addTrueInfoData() / \ref GDataCollection::addDigitizedData() transfer ownership of per-hit objects.
 *   - Storage is \c std::vector<std::unique_ptr<...>> so destruction is automatic.
 * - **Run-level integration**:
 *   - \ref GDataCollection::collectTrueInfosData() / \ref GDataCollection::collectDigitizedData() implement accumulation into a
 *     single "integrated entry" (typically vector size == 1).
 *
 * ## Integration semantics (important)
 * - Truth integration: currently accumulates **double observables only** (strings are not merged).
 * - Digitized integration: accumulates **non-SRO** integer and double observables (which=0) only.
 *   SRO keys are intentionally excluded because they represent per-hit readout coordinates.
 *
 * \note This class is intentionally small and does not enforce invariants such as "truth and
 * digitized must have same hit count". Higher-level code may enforce those policies.
 */

#include "gTrueInfoData.h"
#include "gDigitizedData.h"

// c++
#include <memory>
#include <vector>

/**
 * \brief Per-sensitive-detector container that owns true and digitized hit data.
 *
 * \details
 * A \ref GDataCollection is keyed by sensitive detector name in higher-level containers
 * (e.g. in \ref GEventDataCollection::getDataCollectionMap()).
 *
 * The container stores:
 * - \c trueInfosData  : vector of per-hit truth objects (or size 1 in run-integrated usage)
 * - \c digitizedData  : vector of per-hit digitized objects (or size 1 in run-integrated usage)
 *
 * Two usage modes are supported:
 * - **Event mode**: append one entry per hit (ownership transfer).
 * - **Run mode**: repeatedly integrate hits so the first entry becomes an accumulator.
 */
class GDataCollection
{
public:
	/**
	 * \brief Constructs an empty data collection for a single sensitive detector.
	 *
	 * \details
	 * The detector name itself is stored at higher layers (e.g. as a key in
	 * \ref GEventDataCollection::getDataCollectionMap()).
	 */
	explicit GDataCollection() = default;

	/**
	 * \brief Destructor for \ref GDataCollection.
	 *
	 * \details
	 * All stored hit objects are owned via \c std::unique_ptr and are released automatically.
	 */
	~GDataCollection() = default;

	/**
	 * \brief Integrate ("collect") true-hit data into a run-level aggregate entry.
	 *
	 * \details
	 * Intended for run-level accumulation:
	 * - On the first call, a deep copy of \p data is stored as the first element.
	 * - On subsequent calls, each **double** observable from \p data is accumulated into the
	 *   first element via \ref GTrueInfoData::accumulateVariable().
	 *
	 * Notes / design choices:
	 * - Only the **double** observables are accumulated here.
	 * - String observables are treated as per-hit metadata and are not merged.
	 * - This method performs summation; if you need averages, normalize in the consumer.
	 *
	 * \param data Source true-hit object (not owned; values are copied/accumulated).
	 */
	void collectTrueInfosData(const std::unique_ptr<GTrueInfoData>& data) {
		// first event
		if (trueInfosData.empty()) {
			trueInfosData.push_back(std::make_unique<GTrueInfoData>(*data));
		}
		else {
			for (const auto& [varName, value] : data->getDoubleVariablesMap()) {
				trueInfosData.front()->accumulateVariable(varName, value);
			}
		}
	}

	/**
	 * \brief Integrate ("collect") digitized data into a run-level aggregate entry.
	 *
	 * \details
	 * Intended for run-level accumulation:
	 * - On the first call, a deep copy of \p data is stored as the first element.
	 * - On subsequent calls, integer and double observables are accumulated into the first
	 *   element via \ref GDigitizedData::accumulateVariable().
	 *
	 * Filtering policy:
	 * - This method uses \c which=0 when reading observables maps, i.e. it excludes
	 *   streaming readout keys (crate/slot/channel/timeAtElectronics/chargeAtElectronics).
	 *
	 * Rationale:
	 * - SRO keys identify per-hit readout coordinates and are not meaningful to sum across hits.
	 *
	 * \param data Source digitized-hit object (not owned; values are copied/accumulated).
	 */
	void collectDigitizedData(const std::unique_ptr<GDigitizedData>& data) {
		// first event
		if (digitizedData.empty()) {
			digitizedData.push_back(std::make_unique<GDigitizedData>(*data));
		}
		else {
			// argument passed: 0: do not get SRO var
			for (const auto& [varName, value] : data->getIntObservablesMap(0)) {
				digitizedData.front()->accumulateVariable(varName, value);
			}
			for (const auto& [varName, value] : data->getDblObservablesMap(0)) {
				digitizedData.front()->accumulateVariable(varName, value);
			}
		}
	}

	/**
	 * \brief Add one digitized-hit entry (event-level ownership transfer).
	 *
	 * \details
	 * Event-level API: each hit corresponds to one \ref GDigitizedData object that is owned
	 * by this collection after insertion.
	 *
	 * \param data Digitized-hit object; ownership is moved into this collection.
	 */
	void addDigitizedData(std::unique_ptr<GDigitizedData> data) {
		digitizedData.push_back(std::move(data)); // taking ownership of the unique_ptr
	}

	/**
	 * \brief Add one true-hit entry (event-level ownership transfer).
	 *
	 * \details
	 * Event-level API: each hit corresponds to one \ref GTrueInfoData object that is owned
	 * by this collection after insertion.
	 *
	 * \param data True-hit object; ownership is moved into this collection.
	 */
	void addTrueInfoData(std::unique_ptr<GTrueInfoData> data) {
		trueInfosData.push_back(std::move(data)); // taking ownership of the unique_ptr
	}

	/**
	 * \brief Read-only access to stored true-hit data.
	 *
	 * \details
	 * Interpretation depends on usage:
	 * - Event-level: vector size == number of hits stored for the detector.
	 * - Run-level: vector size is typically 1 (the integrated entry).
	 *
	 * \return Const reference to the owned vector of true-hit objects.
	 */
	[[nodiscard]] auto getTrueInfoData() const -> const std::vector<std::unique_ptr<GTrueInfoData>>& {
		return trueInfosData;
	}

	/**
	 * \brief Read-only access to stored digitized-hit data.
	 *
	 * \details
	 * Interpretation depends on usage:
	 * - Event-level: vector size == number of hits stored for the detector.
	 * - Run-level: vector size is typically 1 (the integrated entry).
	 *
	 * \return Const reference to the owned vector of digitized-hit objects.
	 */
	[[nodiscard]] auto getDigitizedData() const -> const std::vector<std::unique_ptr<GDigitizedData>>& {
		return digitizedData;
	}

private:
	/**
	 * \brief True-hit entries for this detector.
	 *
	 * \details
	 * - Event usage: each element corresponds to one hit (vector index == hit index).
	 * - Run usage: vector usually has one element that acts as an accumulator.
	 */
	std::vector<std::unique_ptr<GTrueInfoData>> trueInfosData;

	/**
	 * \brief Digitized-hit entries for this detector.
	 *
	 * \details
	 * - Event usage: each element corresponds to one hit.
	 * - Run usage: vector usually has one element that acts as an accumulator.
	 */
	std::vector<std::unique_ptr<GDigitizedData>> digitizedData;
};
