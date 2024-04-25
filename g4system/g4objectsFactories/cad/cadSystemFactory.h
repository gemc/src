#ifndef G4CADSYSTEMFACTORY_H
#define G4CADSYSTEMFACTORY_H 1

// g4system
#include "../../g4systemConventions.h"
#include "../g4objectsFactory.h"

#include "gutilities.h"

// system factory
class G4CadSystemFactory : G4ObjectsFactory
{
public:
	bool loadG4System(GOptions* gopt, GVolume *s, map<string, G4Volume*> *g4s) {

		int verbosity = gopt->getInt(G4SYSTEMVERBOSITY);
		string vname = s->getG4Name();

		if(verbosity >= GVERBOSITY_DETAILS) {
			G4cout << G4SYSTEMLOGHEADER << "Importing cad volumes <" << vname << ">" << G4endl;
		}

		G4VSolid*          sbuild = buildSolid(gopt, s, g4s);
		G4LogicalVolume*   lbuild = buildLogical(gopt, s, g4s);
		if ( lbuild != nullptr) {
			lbuild->SetVisAttributes(createVisualAttributes(s));
		}
		G4VPhysicalVolume* pbuild = buildPhysical(gopt, s, g4s);

		if(verbosity >= GVERBOSITY_DETAILS) {
			string solid = sbuild != nullptr ? " solid "    + string(KGRN) + "build," + string(RST) : " solid "   + string(KRED) + "not build," + string(RST);
			string logic = lbuild != nullptr ? " logical "  + string(KGRN) + "build," + string(RST) : " logical " + string(KRED) + "not build," + string(RST);
			string physi = pbuild != nullptr ? " physical " + string(KGRN) + "build," + string(RST) : " physical "+ string(KRED) + "not build," + string(RST);
			G4cout << G4SYSTEMLOGHEADER << "g4volume <" << vname << "> " << solid << logic << physi << " with pointers: " << sbuild << ", " << lbuild << ", " << pbuild << G4endl;
		}

		if(sbuild && lbuild && pbuild) {
			return true;
		} else {
			return false;
		}
	}

private:
	G4VSolid*          buildSolid(   GOptions* gopt, GVolume *s, map<string, G4Volume*> *g4s);
	G4LogicalVolume*   buildLogical( GOptions* gopt, GVolume *s, map<string, G4Volume*> *g4s);
	G4VPhysicalVolume* buildPhysical(GOptions* gopt, GVolume *s, map<string, G4Volume*> *g4s);

};


#endif
