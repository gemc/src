/**
 * \file gdynamicdigitization.cc
 * \brief Implementation of GDynamicDigitization and helpers.
 *
 * Public API documentation is authoritative in gdynamicdigitization.h.
 * This file intentionally avoids duplicating Doxygen function blocks and contains only
 * implementation-level comments.
 *
 * \author Maurizio Ungaro
 * \date 2025-08-08
 */

#include "gdynamicdigitization.h"

// gemc
#include "gtranslationTableConventions.h"
#include "gdataConventions.h"
#include "gtouchableConventions.h"

// c++
#include <iostream>

// See header for API docs.
std::unique_ptr<GTrueInfoData> GDynamicDigitization::collectTrueInformationImpl(GHit* ghit, size_t hitn) {
	auto trueInfoData = std::make_unique<GTrueInfoData>(gopts, ghit);

	std::vector<GIdentifier> identities = ghit->getGID();

	// Include all identities first so they are always present in the record.
	for (auto& identity : identities) {
		trueInfoData->includeVariable(identity.getName(), identity.getValue());
	}

	// Bit 0 is expected to contain the always-present true-hit quantities.
	ghit->calculateInfosForBit(0);

	// Average positions are computed at the hit level by GHit and returned here.
	G4ThreeVector avgGlobalPos = ghit->getAvgGlobaPosition();
	G4ThreeVector avgLocalPos  = ghit->getAvgLocalPosition();

	trueInfoData->includeVariable("totalEDeposited", ghit->getTotalEnergyDeposited());
	trueInfoData->includeVariable("avgTime", ghit->getAverageTime());
	trueInfoData->includeVariable("avgx", avgGlobalPos.getX());
	trueInfoData->includeVariable("avgy", avgGlobalPos.getY());
	trueInfoData->includeVariable("avgz", avgGlobalPos.getZ());
	trueInfoData->includeVariable("avglx", avgLocalPos.getX());
	trueInfoData->includeVariable("avgly", avgLocalPos.getY());
	trueInfoData->includeVariable("avglz", avgLocalPos.getZ());
	trueInfoData->includeVariable("hitn", static_cast<int>(hitn)); // assume hitn < INT_MAX

	// Bit 1 typically includes metadata like the process name.
	trueInfoData->includeVariable("processName", ghit->getProcessName());

	return trueInfoData;
}

// See header for API docs.
void GDynamicDigitization::chargeAndTimeAtHardware(int time, int q, const GHit* ghit, GDigitizedData& gdata) {
	check_if_log_defined();

	if (translationTable == nullptr) {
		log->error(EC__TTNOTFOUNDINTT, "Translation Table not found");
		return;
	}

	// Translate a TT id into a crate/slot/channel triple.
	std::vector<int> haddress = translationTable->getElectronics(ghit->getTTID()).getHAddress();

	// The translation table uses a sentinel to indicate an uninitialized hardware address.
	if (haddress.front() == UNINITIALIZEDNUMBERQUANTITY) {
		log->error(EC__GIDENTITYNOTFOUNDINTT, "Translation Table found, but haddress was not initialized");
		return;
	}

	gdata.includeVariable(CRATESTRINGID, haddress[0]);
	gdata.includeVariable(SLOTSTRINGID, haddress[1]);
	gdata.includeVariable(CHANNELSTRINGID, haddress[2]);
	gdata.includeVariable(TIMEATELECTRONICS, time);
	gdata.includeVariable(CHARGEATELECTRONICS, q);
}

// See header for API docs.
GTouchableModifiers::GTouchableModifiers(const std::vector<std::string>& touchableNames) {
	// Pre-declare keys so later access is well-defined for known touchables.
	for (const auto& tname : touchableNames) {
		modifierWeightsMap[tname] = {};
	}
}

// See header for API docs.
void GTouchableModifiers::insertIdAndWeight(const std::string& touchableName, int idValue, double weight) {
	// Stored as a flat vector: (id, weight, id, weight, ...)
	modifierWeightsMap[touchableName].push_back(idValue);
	modifierWeightsMap[touchableName].push_back(weight);
}

// See header for API docs.
void GTouchableModifiers::insertIdWeightAndTime(const std::string& touchableName, int idValue, double weight,
                                                double             time) {
	// Stored as a flat vector: (id, weight, time, id, weight, time, ...)
	modifierWeightsAndTimesMap[touchableName].push_back(idValue);
	modifierWeightsAndTimesMap[touchableName].push_back(weight);
	modifierWeightsAndTimesMap[touchableName].push_back(time);
}

// See header for API docs.
void GTouchableModifiers::assignOverallWeight(const std::string& tname, double totalWeight) {
	// Normalize (id, weight) pairs.
	size_t countWeights = modifierWeightsMap[tname].size() / 2;
	for (size_t h = 0; h < countWeights; h++) {
		modifierWeightsMap[tname][h * 2 + 1] = modifierWeightsMap[tname][h * 2 + 1] / totalWeight;
	}

	// Normalize (id, weight, time) triplets.
	size_t countWeightsAndTimes = modifierWeightsAndTimesMap[tname].size() / 3;
	for (size_t h = 0; h < countWeightsAndTimes; h++) {
		modifierWeightsAndTimesMap[tname][h * 3 + 1] = modifierWeightsAndTimesMap[tname][h * 3 + 1] / totalWeight;
	}
}

// See header for API docs.
double GDynamicDigitization::processStepTimeImpl([[maybe_unused]] const std::shared_ptr<GTouchable>& gTouchID,
                                                 [[maybe_unused]] G4Step*                            thisStep) {
	// Default time definition: global time of the post-step point.
	return thisStep->GetPostStepPoint()->GetGlobalTime();
}

// See header for API docs.
std::vector<std::shared_ptr<GTouchable>> GDynamicDigitization::processTouchableImpl(
	std::shared_ptr<GTouchable> gtouchable, G4Step* thisStep) {
	double stepTimeAtElectronics      = processStepTime(gtouchable, thisStep);
	int    stepTimeAtElectronicsIndex = readoutSpecs->timeCellIndex(stepTimeAtElectronics);

	std::vector<std::shared_ptr<GTouchable>> result;

	// If the touchable does not yet have a time index, or it matches the current step's
	// index, we can reuse it.
	if (stepTimeAtElectronicsIndex == gtouchable->getStepTimeAtElectronicsIndex() ||
		gtouchable->getStepTimeAtElectronicsIndex() == GTOUCHABLEUNSETTIMEINDEX) {
		gtouchable->assignStepTimeAtElectronicsIndex(stepTimeAtElectronicsIndex);
		result.emplace_back(gtouchable);
	}
	else {
		// Otherwise, create a cloned touchable with the new time index and return both.
		auto cloned = std::make_shared<GTouchable>(gtouchable, stepTimeAtElectronicsIndex);

		// std::initializer_list requires copyable elements, so build the vector explicitly.
		result.emplace_back(gtouchable);
		result.emplace_back(cloned);
	}

	return result;
}

// See header for API docs.
std::vector<std::shared_ptr<GTouchable>> GDynamicDigitization::processGTouchableModifiersImpl(
	[[maybe_unused]] const std::shared_ptr<GTouchable>& gTouchID,
	[[maybe_unused]] const GTouchableModifiers&         gmods) {
	// Default behavior: no modifier processing.
	std::vector<std::shared_ptr<GTouchable>> touchables;
	return touchables;
}
