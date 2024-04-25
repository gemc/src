// g4system
#include "cadSystemFactory.h"

// geant4
#include "G4PVPlacement.hh"

// TODO: print everything about this volume if verbosity
G4VPhysicalVolume* G4CadSystemFactory::buildPhysical(GOptions* gopt, GVolume *s, map<string, G4Volume*> *g4s)
{
    if ( ! s->getExistence() ) {
        return nullptr;
    }
	string g4name = s->getG4Name();
	bool verbosity = getVerbosity(gopt, g4name);

	// check dependencies first
	// gvolume
	// logical volume must exist
	// logical mother must exist
	if(!checkPhysicalDependencies(verbosity, s, g4s)) return nullptr;

	// if it's a component, do nothing
	string matName = s->getMaterial();

	// how to handle components?
	//if(matName == "component") return true;



	// First  G4 Constructor
	//	G4PVPlacement(G4RotationMatrix *pRot,
	//				  const G4ThreeVector &tlate,
	//				  G4LogicalVolume *pCurrentLogical,
	//				  const G4String& pName,
	//				  G4LogicalVolume *pMotherLogical,
	//				  G4bool pMany,
	//				  G4int  pCopyNo,
	//				  G4bool pSurfChk=false);
	// Initialise a single volume, positioned in a frame which is rotated by
	// *pRot and traslated by tlate, relative to the coordinate system of the
	// mother volume pMotherLogical.
	// If pRot=0 the volume is unrotated with respect to its mother.
	// The physical volume is added to the mother's logical volume.
	// Arguments particular to G4PVPlacement:
	//   pMany Currently NOT used. For future use to identify if the volume
	//         is meant to be considered an overlapping structure, or not.
	//   pCopyNo should be set to 0 for the first volume of a given type.
	//   pSurfChk if true activates check for overlaps with existing volumes.
	// This is a very natural way of defining a physical volume, and is
	// especially useful when creating subdetectors: the mother volumes are
	// not placed until a later stage of the assembly program.

	bool checkForOverlaps = false;
	if(gopt->getInt("checkOverlaps") > 0) checkForOverlaps = true;

	G4Volume *thisG4Volume = (*g4s)[g4name];

	if(thisG4Volume->getPhysical() == nullptr) {
		thisG4Volume->setPhysical(new G4PVPlacement(getRotation(s),
																  getPosition(s),
																  thisG4Volume->getLogical(),
																  g4name,
																  getLogicalFromMap(s->getG4MotherName(), g4s),
																  false,
																  s->getPCopyNo(),
																  checkForOverlaps
																  ), verbosity
										  );
	}

	return thisG4Volume->getPhysical();
}


