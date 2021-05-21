// gemc
#include "gsd.h"
#include "gemcConventions.h"

// this is thread-local
GSensitiveDetector::GSensitiveDetector(string sdName, GOptions* goptions, map<string, GDynamicDigitization*> *gDDGlobal) :
G4VSensitiveDetector(sdName),                             // geant4 derived
GStateMessage(goptions, "GSensitiveDetector " + sdName),  // GStateMessage derived
gDynamicDigitizationMapGlobalInstance(gDDGlobal),
eventData(nullptr)
{
	verbosity = goptions->getInt("gsensitivityv");

	// protected, from G4VSensitiveDetector: it's a G4CollectionNameVector
	// not really used in gemc but it's no overhead to do it as in the examples
	collectionName.insert(sdName);

	// should run loadReadoutSpecs here?

	logSummary("Instantiating GSensitiveDetector " + sdName);
}


// thread local
// run at the beginning of the event
void GSensitiveDetector::Initialize(G4HCofThisEvent* g4hc)
{
	string sdName = GetName();
	logSummary("Initialize GSensitiveDetector " + sdName);

	// assinging thread local DynamicDigitization from the global map
	if(gDynamicDigitizationMapGlobalInstance->find(sdName) != gDynamicDigitizationMapGlobalInstance->end()) {
		gDynamicDigitizationLocalInstance = (*gDynamicDigitizationMapGlobalInstance)[sdName];
	}

	// protecting against pluging loading failures
	if(!gDynamicDigitizationLocalInstance) {
		logError(" gDynamicDigitizationLocalInstance for " + sdName + " could not loaded.");
		gexit(EC__DYNAMICPLUGINNOTFOUND);
		return;
	}



}



// thread local
G4bool GSensitiveDetector::ProcessHits(G4Step* thisStep, G4TouchableHistory* g4th)
{
	return true;
}




// thread local
void GSensitiveDetector::EndOfEvent(G4HCofThisEvent* g4hc)
{
	logSummary("EndOfEvent of GSensitiveDetector " + GetName());
}



void GSensitiveDetector::registerGVolumeTouchable(string name, GTouchable* gt)
{
	if(verbosity >= GVERBOSITY_DETAILS) {
		G4cout << "Registering touchable gvolume <" << name << "> with  value: " << gt << G4endl;
	}
	gTouchableMap[name] = gt;
}


