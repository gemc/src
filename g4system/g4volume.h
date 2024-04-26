#ifndef  G4VOLUME_H
#define  G4VOLUME_H 1

// geant4
#include "G4VSolid.hh"
#include "G4LogicalVolume.hh"

class G4Volume
{

private:
   // [[maybe_unused]] int verbosity;
	G4VSolid*             solidVolume;   // Solid Volume
	G4LogicalVolume*    logicalVolume;   // Logical Volume
	G4VPhysicalVolume* physicalVolume;   // Physical Volume

public:
	G4VSolid*          getSolid()    const { if(solidVolume)    return solidVolume;    else return nullptr;}
	G4LogicalVolume*   getLogical()  const { if(logicalVolume)  return logicalVolume;  else return nullptr;}
	G4VPhysicalVolume* getPhysical() const { if(physicalVolume) return physicalVolume; else return nullptr;}

	void setSolid(G4VSolid* s, int verb);
	void setLogical(G4LogicalVolume* l, int verb);
	void setPhysical(G4VPhysicalVolume* p, int verb);

};



#endif
