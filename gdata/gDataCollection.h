#pragma once

/**
 * \file GDataCollection.h
 * \brief Defines the GDataCollection class for storing hit data for a detector.
 *
 * A \ref GDataCollection is the per-sensitive-detector container used by both:
 * - \ref GEventDataCollection (event-level): stores many hits (one entry per hit)
 * - \ref GRunDataCollection   (run-level): integrates hits/events into a single entry
 *
 * It holds two independent collections:
 * - \ref GTrueInfoData "truth" observables derived from simulation / Geant4 tracking.
 * - \ref GDigitizedData "digitized" observables produced by electronics/digitization logic.
 *
 * Ownership model:
 * - Event-level: \c addTrueInfoData / \c addDigitizedData move in unique ownership for each hit.
 * - Run-level integration: \c collectTrueInfosData / \c collectDigitizedData copy/accumulate into a single entry.
 */

#include "gTrueInfoData.h"
#include "gDigitizedData.h"

// c++
#include <memory>
#include <vector>

class GDataCollection
{
public:
	/**
	 * \brief Constructs an empty data collection for a single sensitive detector.
	 *
	 * The detector name itself is stored at higher layers (e.g. as a key in
	 * \ref GEventDataCollection::getDataCollectionMap()).
	 */
	explicit GDataCollection() = default;

	/**
	 * \brief Destructor for GDataCollection.
	 *
	 * All stored hit objects are owned via \c std::unique_ptr and are released automatically.
	 */
	~GDataCollection() = default;

	/**
	 * \brief Integrate ("collect") true-hit data into a run-level aggregate entry.
	 *
	 * This method is intended for run-level accumulation:
	 * - On the first call, a deep copy of \p data is stored as the first element.
	 * - On subsequent calls, each double observable from \p data is added to the first element
	 *   via \ref GTrueInfoData::accumulateVariable().
	 *
	 * Notes:
	 * - Only the double observables are accumulated here (string observables are not merged).
	 * - This method performs summation; if you need averages, normalize later.
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
	 * This method is intended for run-level accumulation:
	 * - On the first call, a deep copy of \p data is stored as the first element.
	 * - On subsequent calls, integer and double observables are accumulated into the first
	 *   element via \ref GDigitizedData::accumulateVariable().
	 *
	 * Filtering:
	 * - This method uses \c which=0 when reading observables maps, i.e. it excludes
	 *   streaming readout keys (crate/slot/channel/timeAtElectronics/chargeAtElectronics).
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
	 * This is the event-level API: each hit becomes an element in \c digitizedData.
	 *
	 * \param data Digitized-hit object; ownership is moved into this collection.
	 */
	void addDigitizedData(std::unique_ptr<GDigitizedData> data) {
		digitizedData.push_back(std::move(data)); // taking ownership of the unique_ptr
	}

	/**
	 * \brief Add one true-hit entry (event-level ownership transfer).
	 *
	 * This is the event-level API: each hit becomes an element in \c trueInfosData.
	 *
	 * \param data True-hit object; ownership is moved into this collection.
	 */
	void addTrueInfoData(std::unique_ptr<GTrueInfoData> data) {
		trueInfosData.push_back(std::move(data)); // taking ownership of the unique_ptr
	}

	/**
	 * \brief Read-only access to stored true-hit data.
	 *
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
	 * For event data, the vector index corresponds to the hit index.
	 * For run-integrated data, the vector typically contains a single aggregated entry.
	 */
	std::vector<std::unique_ptr<GTrueInfoData>> trueInfosData;

	/**
	 * \brief Digitized-hit entries for this detector.
	 *
	 * For event data, the vector index corresponds to the hit index.
	 * For run-integrated data, the vector typically contains a single aggregated entry.
	 */
	std::vector<std::unique_ptr<GDigitizedData>> digitizedData;
};
