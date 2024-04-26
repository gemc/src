// guts
#include "gutilities.h"

// g4system
#include "../g4systemConventions.h"
#include "g4objectsFactory.h"


bool G4ObjectsFactory::checkSolidDependencies(bool verbosity, GVolume *s, map<string, G4Volume*> *g4s)
{
	// checking if it's a copy, replica or solid operation
	// they are mutually exclusive
	string copyOf    = s->getCopyOf();
	string replicaOf = s->getReplicaOf();
	string solidsOpr = s->getSolidsOpr();

	// copy
	if(copyOf != UNINITIALIZEDSTRINGQUANTITY) {
		vector<string> copies = gutilities::getStringVectorFromString(copyOf);
		if(copies.size() == 2) {
			// first string must be copyOf
			if(copies[0] == "copyOf:") {
				// checking if the copy solid exists
				if(verbosity) G4cout << G4SYSTEMLOGHEADER << "<" << s->getName() << "> is a copy of <" << copies[1] << "<" ;
				if(getSolidFromMap(copies[1], g4s) != nullptr) {
					if(verbosity) G4cout << " which already exists" << G4endl; ;
					return true;
				} else {
					if(verbosity) G4cout << " which does not exist yet." << G4endl; ;
					return false;
				}
			} else {
				if(verbosity) G4cout << " Did you intend to make a copy? The first string must be: \"copyOf:\" but you have " << copies[0] << G4endl ;
				return false;
			}
		} else {
			if(verbosity) G4cout << " Did you intend to make a copy? The syntax is \"copyOf:\" volumeName, but you have " << copyOf << G4endl; ;
			return false;
		}
		// replica
	} else if(replicaOf != UNINITIALIZEDSTRINGQUANTITY) {

		// solid operation
	} else if(solidsOpr != UNINITIALIZEDSTRINGQUANTITY) {
		vector<string> solidOperations = gutilities::getStringVectorFromString(solidsOpr);
		if(solidOperations.size() == 3) {
			if(solidOperations[1] == "+" || solidOperations[1] == "-" || solidOperations[1] == "*") {
				// checking if the copy solid exists
				if(getSolidFromMap(solidOperations[0], g4s) != nullptr && getSolidFromMap(solidOperations[2], g4s) != nullptr) return true;
			} else return false;

		} else return false;
	}

	return true;
}

bool G4ObjectsFactory::checkLogicalDependencies([[maybe_unused]] bool verbosity, [[maybe_unused]] GVolume *s, [[maybe_unused]] map<string, G4Volume*> *g4s)
{
	// PRAGMA TODO
	// check material here

	return true;
}

bool G4ObjectsFactory::checkPhysicalDependencies(bool verbosity, GVolume *s, map<string, G4Volume*> *g4s)
{
	string vname      = s->getG4Name();
	string motherName = s->getG4MotherName();

	// the gvolume must exist
	if(g4s->find(vname) == g4s->end()) {

		if(verbosity) G4cout << G4SYSTEMLOGHEADER << "dependencies: " << vname << " not found in gvolume map yet." << G4endl ;

		return false;
	}

	// checking if the logical volume exists
	if(getLogicalFromMap(vname, g4s)   == nullptr) {
		if(verbosity) G4cout << "  <" << vname << ">  - logical volume not found yet." << G4endl ;
		return false;
	}

	// checking if its mother's logical volume exists
	if(motherName != MOTHEROFUSALL && getLogicalFromMap(motherName, g4s) == nullptr) {
		if(verbosity) G4cout << G4SYSTEMLOGHEADER << "dependencies: <" << vname << "> mother <" << motherName << "> logical volume not found yet." << G4endl ;
		return false;
	}

	if(motherName != MOTHEROFUSALL) {
	if(verbosity) G4cout << G4SYSTEMLOGHEADER << "dependencies: <" << vname << "> and mother <" << motherName << "> logical volumes are found. Ready to build or get physical volume." << G4endl ;
	} else {
		if(verbosity) G4cout << G4SYSTEMLOGHEADER << "dependencies: <" << vname << "> logical volume is found. Ready to build or get physical volume." << G4endl ;
	}
	return true;
}
