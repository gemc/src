// gphysics 
#include "gphysics.h"
#include "gphysicsConventions.h"

// glibrary
#include "gutilities.h"
using namespace gutilities;

// geant4 version
#include "G4Version.hh"

// c++
using namespace std;

// geant4

// From examples/extended/physicslists/extensibleFactory:
// The extensible factory also allows for the extension of lists by adding
// (using "+" as a separator) or replacing (using "_" as a separator)
// specific physics constructors.  These can be specified by
// pre-defining a short name (e.g. RADIO for G4RadioactiveDecayPhysics) or
// providing the full class name.
#include "G4PhysListFactoryAlt.hh"

// allow ourselves to extend the short names for physics ctor addition/replace
// along the same lines as EMX, EMY, etc
#include "G4PhysListRegistry.hh"

// allow ourselves to give the user extra info about available physics constructors (ctors)
#include "G4PhysicsConstructorFactory.hh"


GPhysics::GPhysics(GOptions* gopts) : physList(nullptr) {
	
//	int verbosity  = gopts->getInt(GPHYSVERBOSITY);
	bool showPhys    = gopts->getSwitch("showAvailablePhysics");
	bool showPhysX   = gopts->getSwitch("showAvailablePhysicsX");
	string gphysList = gopts->getString("physicsList");

	if ( showPhys || showPhysX ) {
		printAvailable();
	}
	if ( showPhysX ) {
		return ;
	}
	
	// g4alt::G4PhysListFactoryAlt is the extensible factory
	// including the G4PhysListFactoryAlt.hh header and the line:
	//    using namespace g4alt;
	// would make this a drop-in replacement, but we'll list the explicit
	// namespace here just for clarity
	g4alt::G4PhysListFactory factory;
	string g4physList = removeAllSpacesFromString(gphysList);

	physList = factory.GetReferencePhysList(g4physList);

	if ( ! physList ) {		
		cerr << FATALERRORL << "physics list <" << gphysList << "> could not be loaded." << endl;
		gexit(EC__PHYSLISTERROR);
	}
	
	cout << GPHYSLOGHEADER << "Geant4 physics list: <" << g4physList << ">" << endl;
}

GPhysics::~GPhysics() {}


// calls PrintAvailablePhysLists
// if verbosity is > 0 calls PrintAvailablePhysicsConstructors
void GPhysics::printAvailable() {
	
	cout << endl << "Geant4 Version " <<  replaceCharInStringWithChars(G4Version, "$", "") << "  " << G4Date << endl << endl;
	
	g4alt::G4PhysListFactory factory;
	factory.PrintAvailablePhysLists();
	
	G4cout << GPHYSLOGHEADER << " Geant4 available physics constructor that can be added to physicsList:" << G4endl;
	G4PhysicsConstructorRegistry* g4pctorFactory = G4PhysicsConstructorRegistry::Instance();
	g4pctorFactory->PrintAvailablePhysicsConstructors();

}



