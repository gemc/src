/**
* @file   g4volume.cc
 * @ingroup g4system_geometry
 * @brief  Implementation of the small pointer-holder utilities in G4Volume.
 */

#include "g4volume.h"

// Store the solid pointer and emit a debug trace to help diagnose factory/build ordering.
void G4Volume::setSolid(G4VSolid* s, const std::shared_ptr<GLogger>& log) {
	solidVolume = s;
	log->debug(NORMAL, "G4Volume: solid assigned @", s);
}

// Store the logical pointer and emit a debug trace to help diagnose factory/build ordering.
void G4Volume::setLogical(G4LogicalVolume* l, const std::shared_ptr<GLogger>& log) {
	logicalVolume = l;
	log->debug(NORMAL, "G4Volume: logical assigned @", l);
}

// Store the physical pointer and emit a debug trace to help diagnose factory/build ordering.
void G4Volume::setPhysical(G4VPhysicalVolume* p, const std::shared_ptr<GLogger>& log) {
	physicalVolume = p;
	log->debug(NORMAL, "G4Volume: physical assigned @", p);
}

// Attach a field manager to the logical volume if it exists; otherwise do nothing.
void G4Volume::setFieldManager(G4FieldManager* fm, bool forceToAllDaughters) {
	if (!logicalVolume) return; // nothing to attach to
	logicalVolume->SetFieldManager(fm, forceToAllDaughters);
}
