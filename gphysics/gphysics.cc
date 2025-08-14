// gphysics 
#include "gphysics.h"
#include "gphysics_options.h"
#include "gphysicsConventions.h"

// gemc
#include "gutilities.h"

// geant4 version
#include "G4Version.hh"


// geant4

// From examples/extended/physicslists/extensibleFactory:
// The extensible factory also allows for the extension of lists by adding
// (using "+" as a separator) or replacing (using "_" as a separator)
// specific physics constructors.  These can be specified by
// pre-defining a short name (e.g. RADIO for G4RadioactiveDecayPhysics) or
// providing the full class name.
#include "G4PhysListFactoryAlt.hh"

#include "G4StepLimiterPhysics.hh"

// allow ourselves to give the user extra info about available physics constructors (ctors)
#include "G4PhysicsConstructorFactory.hh"


GPhysics::GPhysics(const std::shared_ptr<GOptions>& gopts) :
	GBase(gopts, GPHYSICS_LOGGER),
	physList(nullptr) {
	log->debug(CONSTRUCTOR, "GPhysics");

	bool        showPhys  = gopts->getSwitch("showPhysics");
	std::string gphysList = gopts->getScalarString("phys_list");

	if (showPhys) {
		printAvailable();
		return;
	}


	// g4alt::G4PhysListFactoryAlt is the extensible factory
	// including the G4PhysListFactoryAlt.hh header and the line:
	//    using namespace g4alt;
	// would make this a drop-in replacement, but we'll list the explicit
	// namespace here just for clarity
	g4alt::G4PhysListFactory factory;
	std::string              g4physList = gutilities::removeAllSpacesFromString(gphysList);

	physList = factory.GetReferencePhysList(g4physList);

	// register step limiters
	physList->RegisterPhysics(new G4StepLimiterPhysics());

	if (!physList) { log->error(ERR_PHYSLISTERROR, "physics list <" + gphysList + "> could not be loaded."); }

	log->info(2, "G4PhysListFactory: <" + g4physList + "> loaded.");
}


// calls PrintAvailablePhysLists
// if verbosity is > 0 calls PrintAvailablePhysicsConstructors
void GPhysics::printAvailable() const {
	std::string g4ver = gutilities::replaceCharInStringWithChars(G4Version, "$", "");

	log->info(0, "Geant4 Version ", g4ver, " ", G4Date);;

	g4alt::G4PhysListFactory factory;
	factory.PrintAvailablePhysLists();

	log->info(0, "Available Geant4 Physics Lists:");

	G4PhysicsConstructorRegistry* g4pctorFactory = G4PhysicsConstructorRegistry::Instance();
	g4pctorFactory->PrintAvailablePhysicsConstructors();
}
