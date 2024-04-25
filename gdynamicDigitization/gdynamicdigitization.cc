// gdynamic
#include "gdynamicdigitization.h"

// glibrary
#include "gtranslationTableConventions.h"
#include "gdataConventions.h"
#include "gtouchableConventions.h"

// c++
using std::cerr;
using std::endl;

// notice: if the energy deposited is very low (~50eV)
// the rounding error on the averave calculations could be up to 10^-3
GTrueInfoData* GDynamicDigitization::collectTrueInformation(GHit *ghit, size_t hitn)
{
	GTrueInfoData* trueInfoData = new GTrueInfoData(ghit);


    vector<GIdentifier> identities = ghit->getGID();

    // loop over all identities
    for ( auto& identity: identities ) {
        trueInfoData->includeVariable(identity.getName(), identity.getValue() );
    }


	// notices:
	// we do each var hitbit group loop separately in case some are not filled

	// bit 0: always there
	ghit->calculateInfosForBit(0);
	G4ThreeVector avgGlobalPos = ghit->getAvgGlobaPosition();
	G4ThreeVector avgLocalPos  = ghit->getAvgLocalPosition();

	trueInfoData->includeVariable("totalEDeposited", ghit->getTotalEnergyDeposited());
	trueInfoData->includeVariable("avgTime", ghit->getAverageTime());
	trueInfoData->includeVariable("avgx",    avgGlobalPos.getX());
	trueInfoData->includeVariable("avgy",    avgGlobalPos.getY());
	trueInfoData->includeVariable("avgz",    avgGlobalPos.getZ());
	trueInfoData->includeVariable("avglx",   avgLocalPos.getX());
	trueInfoData->includeVariable("avgly",   avgLocalPos.getY());
	trueInfoData->includeVariable("avglz",   avgLocalPos.getZ());
	trueInfoData->includeVariable("hitn",    hitn);


	// bit 1:
	trueInfoData->includeVariable("processName",  ghit->getProcessName());

	
	return trueInfoData;
}

// this will set the gdata variable TIMEATELECTRONICS used by RunAction to identify the eventFrameIndex
// and will include in gdata the translation table (hardware address crate/slot/channel)
// this will exit with error if the TT is not defined
// notice time is an int (assumed unit: ns)
void GDynamicDigitization::chargeAndTimeAtHardware(int time, int q, GHit *ghit, GDigitizedData *gdata)
{

	// gexit if translation table not defined
	if ( translationTable == nullptr ) {
		cerr << FATALERRORL << "Translation Table not found" << endl;
		gexit(EC__TTNOTFOUNDINTT);

	} else {

		// in order: crate, slot, channel
		vector<int> haddress = translationTable->getElectronics(ghit->getTTID()).getHAddress();

		// gexit if ghit haddress not initialized
		if ( haddress.front() == UNINITIALIZEDNUMBERQUANTITY) {
			cerr << FATALERRORL << "Translation Table found, but haddress was not initialized." << endl;
			gexit(EC__GIDENTITYNOTFOUNDINTT);
		} else {

			// everything is good.
			// include crate/slot/channel address
			// include payload time and charge for this hit

			gdata->includeVariable(CRATESTRINGID,       haddress[0]);
			gdata->includeVariable(SLOTSTRINGID,        haddress[1]);
			gdata->includeVariable(CHANNELSTRINGID,     haddress[2]);
			gdata->includeVariable(TIMEATELECTRONICS,   time);
			gdata->includeVariable(CHARGEATELECTRONICS, q);
		}
	}
}

void GDynamicDigitization::gDLogMessage(std::string message) {
	gLogMessage(gdMessageHeader + message);
}


GTouchableModifiers::GTouchableModifiers(vector<string> touchableNames) {
	for ( auto tname: touchableNames ) {
		modifierWeightsMap[tname] = {} ;
	}
}


void GTouchableModifiers::insertIdAndWeight(string touchableName, int idValue, double weight) {
	modifierWeightsMap[touchableName].push_back(idValue);
	modifierWeightsMap[touchableName].push_back(weight);
}

void GTouchableModifiers::insertIdWeightAndTime(string touchableName, int idValue, double weight, double time) {
	modifierWeightsAndTimesMap[touchableName].push_back(idValue);
	modifierWeightsAndTimesMap[touchableName].push_back(weight);
	modifierWeightsAndTimesMap[touchableName].push_back(time);
}


// normalize a map using totalWeight
void GTouchableModifiers::assignOverallWeight(string tname, double totalWeight) {

	size_t modifierWeightsMapCount = modifierWeightsMap[tname].size() / 2;

	for (size_t h = 0;  h < modifierWeightsMapCount; h++) {
		modifierWeightsMap[tname][h*2+1] = modifierWeightsMap[tname][h*2+1] / totalWeight;
	}

	modifierWeightsMapCount = modifierWeightsAndTimesMap[tname].size() / 2;

	for (size_t h = 0;  h < modifierWeightsMapCount; h++) {
		modifierWeightsAndTimesMap[tname][h*2+1] = modifierWeightsMap[tname][h*2+1] / totalWeight;
	}

}

// if not overloaded, the time used in processTouchable is simply the step time
float GDynamicDigitization:: processStepTime([[maybe_unused]] GTouchable *gTouchID, [[maybe_unused]] G4Step* thisStep) {
	return thisStep->GetPostStepPoint()->GetGlobalTime();
}


// if not overloaded, returns:
// a vector with a single touchable, with cell time index based on processStepTime if the time index is the same as the gTouchID's (or if it wasn't set)
// a vectory with two touchables, one with the original gtouchID time index, and one with the new one
vector<GTouchable*> GDynamicDigitization::processTouchable(GTouchable *gTouchID, G4Step* thisStep) {

	float stepTimeAtElectronics = processStepTime(gTouchID, thisStep);
    float stepTimeAtElectronicsIndex = readoutSpecs->timeCellIndex(stepTimeAtElectronics);

    if ( stepTimeAtElectronicsIndex == gTouchID->getStepTimeAtElectronicsIndex() || gTouchID->getStepTimeAtElectronicsIndex() == GTOUCHABLEUNSETTIMEINDEX) {
        gTouchID->assignStepTimeAtElectronicsIndex(stepTimeAtElectronicsIndex);
        return { gTouchID };
    } else {
        return { gTouchID, new GTouchable(gTouchID, stepTimeAtElectronicsIndex) };
    }

}



vector<GTouchable*> GDynamicDigitization::processGTouchableModifiers([[maybe_unused]] GTouchable *gTouchID, [[maybe_unused]] GTouchableModifiers gmods) {

	vector<GTouchable*> touchables;
	
	
	
	
	

	return touchables;
}
