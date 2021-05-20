// gemc
#include "gsd.h"

// this is thread-local
GSensitiveDetector::GSensitiveDetector(string sdName, GOptions* goptions, map<string, GDynamicDigitization*> *gDDGlobal) :
G4VSensitiveDetector(sdName),                             // geant4 derived
GStateMessage(goptions, "GSensitiveDetector " + sdName),  // GStateMessage derived
gDynamicDigitizationGlobalInstance(gDDGlobal),
eventData(nullptr)
{
	verbosity = goptions->getInt("gsensitivityv");

	// protected from G4VSensitiveDetector: it's a G4CollectionNameVector
	// not really used in gemc but it's no overhead here
	collectionName.insert(sdName);

	// should run loadReadoutSpecs here?

	message("Instantiating GSensitiveDetector " + sdName);
}
