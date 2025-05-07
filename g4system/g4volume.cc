/**
* @file   g4volume.cc
 * @ingroup Geometry
 * @brief  Inline definitions separated for clearer compilation units.
 */

#include "g4volume.h"


void G4Volume::setSolid(G4VSolid* s, const std::shared_ptr<GLogger>& log)
{
	solidVolume = s;
	log->debug(NORMAL, "G4Volume: solid assigned @", s);
}

void G4Volume::setLogical(G4LogicalVolume* l, const std::shared_ptr<GLogger>& log)
{
	logicalVolume = l;
	log->debug(NORMAL, "G4Volume: logical assigned @", l);
}

void G4Volume::setPhysical(G4VPhysicalVolume* p, const std::shared_ptr<GLogger>& log)
{
	physicalVolume = p;
	log->debug(NORMAL, "G4Volume: physical assigned @", p);
}

void G4Volume::setFieldManager(G4FieldManager* fm, bool forceToAllDaughters)
{
	if (!logicalVolume) return;           // nothing to attach to
	logicalVolume->SetFieldManager(fm, forceToAllDaughters);
}

