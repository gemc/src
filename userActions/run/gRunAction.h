#ifndef GRUNACTION_H
#define GRUNACTION_H 1

// geant4
#include "G4UserRunAction.hh"

// gemc
#include "gLog.h"

// mlibrary
#include "goptions.h"
#include "gdynamic.h"
#include "gmedia.h"

class GRunAction : public G4UserRunAction, public GFlowMessage
{
public:
	// constructor and destructor
	GRunAction(GOptions* gopt, map<string, GDynamic*> *gDigitization, map<string, GMedia*> *gmedia);
	virtual ~GRunAction();


private:
	// virtual method from G4UserRunAction.
	virtual G4Run* GenerateRun();
	virtual void BeginOfRunAction(const G4Run*);
	virtual void EndOfRunAction(const G4Run*);
	
	GOptions* gopt;
	
	// digitization map, loaded in main(), passed here
	map<string, GDynamic*> *gDigitizationGlobal;
	
	// output factories map, loaded in GActionInitialization constructor and passed here
	map<string, GMedia*> *gmediaFactory;

};




#endif
