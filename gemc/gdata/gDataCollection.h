#pragma once

/**
 * \file gDataCollection.h
 * \brief Defines the GDataCollection class, the per-detector container of truth and digitized hit data.
 *
 * \details
 * GDataCollection is the detector-local aggregation unit used by higher-level containers.
 * One instance usually corresponds to one sensitive detector entry and stores:
 * - truth-side objects of type GTrueInfoData
 * - digitized objects of type GDigitizedData
 *
 * It supports two distinct usage patterns.
 *
 * Event mode:
 * - append one truth object and/or one digitized object per hit
 * - ownership of inserted objects is transferred to this container
 *
 * Integrated mode:
 * - accumulate many contributions into the first stored entry of each vector
 * - truth accumulation currently merges only floating-point truth variables
 * - digitized accumulation currently merges only non-SRO scalar observables
 *
 * Design notes:
 * - the class does not require truth and digitized vectors to have matching sizes
 * - detector naming is managed by higher-level maps such as those in GEventDataCollection
 *   and GRunDataCollection
 * - vector index meaning depends on whether the object is used in event or integrated mode
 */

#include "gDigitizedData.h"
#include "gTrueInfoData.h"

// c++
#include <memory>
#include <ostream>
#include <vector>

/**
 * \defgroup gdata_detector_collection GData detector collection
 * \brief Detector-local ownership and accumulation of truth and digitized hit objects.
 *
 * \details
 * This topic covers the container that sits between hit-level objects and higher-level event or
 * run aggregators. It is the place where event-style append semantics and integrated accumulation
 * semantics are both implemented at detector scope.
 */

/**
 * \brief Per-sensitive-detector container that owns truth and digitized data objects.
 * \ingroup gdata_detector_collection
 *
 * \details
 * In higher-level maps, a GDataCollection is usually keyed by the sensitive detector name.
 *
 * Stored content:
 * - \c trueInfosData : truth objects, typically one per hit in event mode
 * - \c digitizedData : digitized objects, typically one per hit in event mode
 *
 * Usage modes:
 * - Event mode : vectors grow by appending objects
 * - Integrated mode : the first vector element acts as the running accumulator
 */
class GDataCollection
{
public:
	/**
	 * \brief Constructs an empty detector-local data collection.
	 *
	 * \details
	 * The object starts with empty truth and digitized vectors.
	 * Detector naming is handled by the owning map at higher levels.
	 */
	explicit GDataCollection() = default;

	/**
	 * \brief Destroys the collection and all owned hit objects.
	 *
	 * \details
	 * Both truth and digitized entries are stored in \c std::unique_ptr containers,
	 * so destruction is automatic and exception-safe.
	 */
	~GDataCollection() = default;

	/**
	 * \brief Integrates one truth object into the detector-level accumulator.
	 *
	 * \details
	 * This method is intended for integrated usage.
	 *
	 * Behavior:
	 * - if the truth vector is empty, a deep copy of \p data becomes the first accumulator entry
	 * - otherwise, each numeric truth observable from \p data is summed into the first entry
	 *
	 * Current integration policy:
	 * - only numeric truth observables are accumulated
	 * - string truth observables are not merged
	 *
	 * \param data Source truth object whose values are copied or accumulated.
	 */
	void collectTrueInfosData(const std::unique_ptr<GTrueInfoData>& data) {
		// The first integrated contribution creates the detector-local accumulator entry.
		if (trueInfosData.empty()) {
			trueInfosData.push_back(std::make_unique<GTrueInfoData>(*data));
		}
		else {
			// Subsequent contributions add only numeric observables into the first stored entry.
			for (const auto& [varName, value] : data->getDoubleVariablesMap()) {
				trueInfosData.front()->accumulateVariable(varName, value);
			}
		}
	}

	/**
	 * \brief Integrates one digitized object into the detector-level accumulator.
	 *
	 * \details
	 * This method is intended for integrated usage.
	 *
	 * Behavior:
	 * - if the digitized vector is empty, a deep copy of \p data becomes the first accumulator entry
	 * - otherwise, non-SRO integer and floating-point observables from \p data are summed into
	 *   the first entry
	 *
	 * Current integration policy:
	 * - only scalar observables are accumulated
	 * - SRO keys are excluded by requesting filtered maps with \c which = 0
	 *
	 * \param data Source digitized object whose values are copied or accumulated.
	 */
	void collectDigitizedData(const std::unique_ptr<GDigitizedData>& data) {
		// The first integrated contribution creates the detector-local accumulator entry.
		if (digitizedData.empty()) {
			digitizedData.push_back(std::make_unique<GDigitizedData>(*data));
		}
		else {
			// Only non-SRO scalar observables are accumulated in integrated mode.
			for (const auto& [varName, value] : data->getIntObservablesMap(0)) {
				digitizedData.front()->accumulateVariable(varName, value);
			}
			for (const auto& [varName, value] : data->getDblObservablesMap(0)) {
				digitizedData.front()->accumulateVariable(varName, value);
			}
		}
	}

	/**
	 * \brief Appends one digitized object in event mode.
	 *
	 * \details
	 * Ownership of \p data is transferred to this collection.
	 * Each appended object normally corresponds to one hit for this detector in one event.
	 *
	 * \param data Digitized object to store.
	 */
	void addDigitizedData(std::unique_ptr<GDigitizedData> data) {
		// Event-mode insertion appends the new hit object and transfers ownership into the container.
		digitizedData.push_back(std::move(data));
	}

	/**
	 * \brief Appends one truth object in event mode.
	 *
	 * \details
	 * Ownership of \p data is transferred to this collection.
	 * Each appended object normally corresponds to one hit for this detector in one event.
	 *
	 * \param data Truth object to store.
	 */
	void addTrueInfoData(std::unique_ptr<GTrueInfoData> data) {
		// Event-mode insertion appends the new hit object and transfers ownership into the container.
		trueInfosData.push_back(std::move(data));
	}

	/**
	 * \brief Returns read-only access to the stored truth objects.
	 *
	 * \details
	 * Interpretation depends on the usage mode:
	 * - in event mode, vector size usually matches the number of truth hits for the detector
	 * - in integrated mode, vector usually contains one accumulator entry
	 *
	 * \return Const reference to the owned vector of truth objects.
	 */
	[[nodiscard]] auto getTrueInfoData() const -> const std::vector<std::unique_ptr<GTrueInfoData>>& {
		return trueInfosData;
	}

	/**
	 * \brief Returns read-only access to the stored digitized objects.
	 *
	 * \details
	 * Interpretation depends on the usage mode:
	 * - in event mode, vector size usually matches the number of digitized hits for the detector
	 * - in integrated mode, vector usually contains one accumulator entry
	 *
	 * \return Const reference to the owned vector of digitized objects.
	 */
	[[nodiscard]] auto getDigitizedData() const -> const std::vector<std::unique_ptr<GDigitizedData>>& {
		return digitizedData;
	}

	/**
	 * \brief Returns mutable access to the stored digitized objects.
	 *
	 * \details
	 * This accessor is intended for workflows that need to update detector-local integrated
	 * digitized payloads after collection, such as final normalization at run end.
	 *
	 * \return Mutable reference to the owned vector of digitized objects.
	 */
	[[nodiscard]] auto getMutableDigitizedData() -> std::vector<std::unique_ptr<GDigitizedData>>& {
		return digitizedData;
	}

private:
	/**
	 * \brief Stored truth objects for this detector.
	 *
	 * \details
	 * Event mode:
	 * - one element per hit
	 *
	 * Integrated mode:
	 * - usually one element acting as the accumulator
	 */
	std::vector<std::unique_ptr<GTrueInfoData>> trueInfosData;

	/**
	 * \brief Stored digitized objects for this detector.
	 *
	 * \details
	 * Event mode:
	 * - one element per hit
	 *
	 * Integrated mode:
	 * - usually one element acting as the accumulator
	 */
	std::vector<std::unique_ptr<GDigitizedData>> digitizedData;

protected:
	friend std::ostream& operator<<(std::ostream& os, const GDataCollection& collection) {
		os << "GDataCollection{";

		os << "trueInfosData=[";
		for (size_t i = 0; i < collection.trueInfosData.size(); ++i) {
			if (i != 0) {
				os << ", ";
			}
			if (collection.trueInfosData[i]) {
				os << *collection.trueInfosData[i];
			}
			else {
				os << "null";
			}
		}
		os << "]";

		os << ", digitizedData=[";
		for (size_t i = 0; i < collection.digitizedData.size(); ++i) {
			if (i != 0) {
				os << ", ";
			}
			if (collection.digitizedData[i]) {
				os << *collection.digitizedData[i];
			}
			else {
				os << "null";
			}
		}
		os << "]";

		os << "}";
		return os;
	}
};