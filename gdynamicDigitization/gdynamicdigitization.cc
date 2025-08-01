/**
 * \file gdynamicdigitization.cc
 * \brief Implementation of the GDynamicDigitization interface and related classes.
 *
 * This file implements methods for processing digitization steps, collecting true hit information,
 * applying touchable modifiers, and setting hardware-level charge and time information.
 *
 * \author Your Name
 * \date YYYY-MM-DD
 */

#include "gdynamicdigitization.h"

// gemc
#include "gtranslationTableConventions.h"
#include "gdataConventions.h"
#include "gtouchableConventions.h"

// c++
#include <iostream>

/**
 * \brief Collects true hit information from a GHit.
 *
 * Integrates all available information built in GHit::addHitInfosForBitset (such as energy, time, and positions)
 * into a new GTrueInfoData object.
 *
 * \param ghit Pointer to the GHit.
 * \param hitn Hit index.
 * \return Pointer to a newly allocated GTrueInfoData object.
 */

std::unique_ptr<GTrueInfoData> GDynamicDigitization::collectTrueInformationImpl(GHit* ghit, size_t hitn) {
	auto trueInfoData = std::make_unique<GTrueInfoData>(ghit, data_logger);

	std::vector<GIdentifier> identities = ghit->getGID();

	// Loop over all identities and include them in the true info data.
	for (auto& identity : identities) { trueInfoData->includeVariable(identity.getName(), identity.getValue()); }

	// Process bit 0: always present values.
	ghit->calculateInfosForBit(0);
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
	trueInfoData->includeVariable("hitn", static_cast<int>(hitn)); // reasonable to assume hitn is less than INT_MAX

	// Process bit 1: include process name.
	trueInfoData->includeVariable("processName", ghit->getProcessName());

	return trueInfoData;
}

/**
 * \brief Sets charge and time information at the hardware level in a GDigitizedData object.
 *
 * Checks that the translation table is defined and that the hardware address (from the GHit's TTID)
 * is initialized. Then, it includes the crate, slot, channel, time, and charge in the digitized data.
 *
 * \param time Time value (assumed unit: ns).
 * \param q Charge value.
 * \param ghit Pointer to the GHit.
 * \param gdata Pointer to the GDigitizedData.
 */
void GDynamicDigitization::chargeAndTimeAtHardware(int time, int q, const GHit* ghit, GDigitizedData& gdata) {
	check_if_log_defined();
	// Exit if the translation table is not defined.
	if (translationTable == nullptr) { tt_logger->error(EC__TTNOTFOUNDINTT, "Translation Table not found"); }
	else {
		// Obtain the hardware address (crate, slot, channel) from the translation table.
		std::vector<int> haddress = translationTable->getElectronics(ghit->getTTID()).getHAddress();
		// Exit if the hardware address is not properly initialized.
		if (haddress.front() == UNINITIALIZEDNUMBERQUANTITY) { tt_logger->error(EC__GIDENTITYNOTFOUNDINTT, "Translation Table found, but haddress was not initialized"); }
		else {
			// Include hardware address, time, and charge in the digitized data.
			gdata.includeVariable(CRATESTRINGID, haddress[0]);
			gdata.includeVariable(SLOTSTRINGID, haddress[1]);
			gdata.includeVariable(CHANNELSTRINGID, haddress[2]);
			gdata.includeVariable(TIMEATELECTRONICS, time);
			gdata.includeVariable(CHARGEATELECTRONICS, q);
		}
	}
}

/**
 * \brief Constructs a GTouchableModifiers object.
 *
 * For each touchable name provided, an empty vector is initialized in the modifierWeightsMap.
 *
 * \param touchableNames A vector of touchable names.
 */
GTouchableModifiers::GTouchableModifiers(const std::vector<std::string>& touchableNames) { for (const auto& tname : touchableNames) { modifierWeightsMap[tname] = {}; } }

/**
 * \brief Inserts an (id, weight) pair for a given touchable.
 *
 * \param touchableName The name of the touchable.
 * \param idValue The identifier value.
 * \param weight The weight.
 */
void GTouchableModifiers::insertIdAndWeight(const std::string& touchableName, int idValue, double weight) {
	modifierWeightsMap[touchableName].push_back(idValue);
	modifierWeightsMap[touchableName].push_back(weight);
}

/**
 * \brief Inserts an (id, weight, time) triplet for a given touchable.
 *
 * \param touchableName The name of the touchable.
 * \param idValue The identifier value.
 * \param weight The weight.
 * \param time The time.
 */
void GTouchableModifiers::insertIdWeightAndTime(const std::string& touchableName, int idValue, double weight, double time) {
	modifierWeightsAndTimesMap[touchableName].push_back(idValue);
	modifierWeightsAndTimesMap[touchableName].push_back(weight);
	modifierWeightsAndTimesMap[touchableName].push_back(time);
}

/**
 * \brief Normalizes modifier weights using the provided total weight.
 *
 * For modifierWeightsMap, it divides each weight (second element in each pair)
 * by totalWeight. For modifierWeightsAndTimesMap, it divides each weight (second element in each triplet)
 * by totalWeight.
 *
 * \param tname The touchable name.
 * \param totalWeight The total weight to use for normalization.
 */
void GTouchableModifiers::assignOverallWeight(const std::string& tname, double totalWeight) {
	size_t countWeights = modifierWeightsMap[tname].size() / 2;
	for (size_t h = 0; h < countWeights; h++) { modifierWeightsMap[tname][h * 2 + 1] = modifierWeightsMap[tname][h * 2 + 1] / totalWeight; }
	size_t countWeightsAndTimes = modifierWeightsAndTimesMap[tname].size() / 3; // Triplets: (id, weight, time)
	for (size_t h = 0; h < countWeightsAndTimes; h++) {
		// Normalize the weight in each triplet (index: h*3 + 1).
		modifierWeightsAndTimesMap[tname][h * 3 + 1] = modifierWeightsAndTimesMap[tname][h * 3 + 1] / totalWeight;
	}
}

/**
 * \brief Default implementation of processStepTime.
 *
 * Returns the global time from the post-step point of the current G4Step.
 *
 * \param gTouchID Pointer to the GTouchable (unused in this default implementation).
 * \param thisStep Pointer to the current G4Step.
 * \return The global time.
 */
double GDynamicDigitization::processStepTimeImpl([[maybe_unused]] const std::shared_ptr<GTouchable>& gTouchID, [[maybe_unused]] G4Step* thisStep) {
	return thisStep->GetPostStepPoint()->GetGlobalTime();
}

/**
 * \brief Processes a GTouchable based on the current G4Step.
 *
 * Computes the time cell index using the readout specifications. If the computed
 * index is equal to the current touchable's index (or if not set), assigns it and returns the original touchable.
 * Otherwise, returns a vector with the original touchable and a new one with the updated index.
 *
 * \param gtouchable Pointer to the original GTouchable.
 * \param thisStep Pointer to the current G4Step.
 * \return A vector of GTouchable pointers.
 */
std::vector<std::shared_ptr<GTouchable>> GDynamicDigitization::processTouchableImpl(std::shared_ptr<GTouchable> gtouchable, G4Step* thisStep) {

	double stepTimeAtElectronics      = processStepTime(gtouchable, thisStep);
	int    stepTimeAtElectronicsIndex = readoutSpecs->timeCellIndex(stepTimeAtElectronics);

	std::vector<std::shared_ptr<GTouchable>> result;

	if (stepTimeAtElectronicsIndex == gtouchable->getStepTimeAtElectronicsIndex() ||
	    gtouchable->getStepTimeAtElectronicsIndex() == GTOUCHABLEUNSETTIMEINDEX) {
		gtouchable->assignStepTimeAtElectronicsIndex(stepTimeAtElectronicsIndex);

		result.emplace_back(gtouchable);
	}
	else {
		// Create a new GTouchable with the updated time index.
		auto cloned = std::make_shared<GTouchable>(gtouchable, stepTimeAtElectronicsIndex);

		// release ownership of the original touchable and return both.
		// std::initializer_list requires copyable elements, so we need to create the vector first
		result.emplace_back(gtouchable);
		result.emplace_back(cloned);
	}

	return result;
}

/**
 * \brief Default implementation for processing touchable modifiers.
 *
 * Returns an empty vector; derived classes may override to apply actual modifications.
 *
 * \param gTouchID Pointer to the original GTouchable.
 * \param gmods A GTouchableModifiers object.
 * \return An empty vector of GTouchable pointers.
 */
// TODO: are we using this anywhere? pass readonly touchId or move it?
std::vector<std::shared_ptr<GTouchable>> GDynamicDigitization::processGTouchableModifiersImpl([[maybe_unused]] const std::shared_ptr<GTouchable>& gTouchID,
                                                                                              [[maybe_unused]] const GTouchableModifiers&         gmods) {
	std::vector<std::shared_ptr<GTouchable>> touchables;

	// touchables.emplace_back(std::make_unique<GTouchable>(*gTouchID, 1)); // Ensure we have a valid touchable to return

	return touchables;
}
