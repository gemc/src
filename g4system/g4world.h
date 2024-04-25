#ifndef  G4WORLD_H
#define  G4WORLD_H 1

// glibrary
#include "gworld.h"

// g4system
#include "g4volume.h"


// a world is a collection of GSystem, their
class G4World
{
public:
	// constructor from a GWorld, using verbosity
	G4World(GWorld *gworld, GOptions* gopts);

	~G4World() {
		// the deletion of g4 volumes and materials
		// is done by G4RunManager, we should not do it in the destructor
		//		for (auto& [keys, values]: (*g4volumesMap) ) {
		//			delete values;
		//		}

		delete g4volumesMap;
		delete g4materialsMap;
	}

private:
	// the key has the form gsystem/gvolumename
	map<string, G4Volume*> *g4volumesMap;

	// the point in filling the map is to create the g4materials
	// so they are available in G4NistManager
	map<string, G4Material*> *g4materialsMap;


	string g4FactoryNameFromSystemFactory(string factory);

	// return false if the material cannot be added
	// for example, if the components to do exist G4NistManager
	bool createG4Material(const GMaterial *gmaterial, int verbosity);

	void buildDefaultMaterialsElementsAndIsotopes(int verbosity);


public:
	inline const G4Volume* getG4Volume(string volumeName) const {

		if ( g4volumesMap->find(volumeName) != g4volumesMap->end() ) {
			return (*g4volumesMap)[volumeName];
		}
		return nullptr;
	}


};


#endif

