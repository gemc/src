#ifndef  GSENSITIVEDETECTOR_H
#define  GSENSITIVEDETECTOR_H 1



// gemc
#include "gStateMessage.h"

// geant4
#include "G4VSensitiveDetector.hh"

// glibrary
#include "goptions.h"
#include "gdynamicdigitization.h"


class GSensitiveDetector : public G4VSensitiveDetector, public GStateMessage
{

public:
	GSensitiveDetector(string sdName, GOptions* goptions, map<string, GDynamicDigitization*> *gDDGlobal);



private:
	int verbosity;

	// the GSensitiveDetector is built before the digitization, so we need
	// a pointer to global digitization map so we can pick gDigiLocal at initialization
	// it will be loaded later with the plugins
	map<string, GDynamic*> *gDynamicDigitizationGlobalInstance;

	GEventData *eventData;

};

#endif

