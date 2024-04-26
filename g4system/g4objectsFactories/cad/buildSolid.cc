// g4system
#include "cadSystemFactory.h"

// Cadmesh single header library
// https://github.com/christopherpoole/CADMesh
// Modifications: had to mark these 2 classes 'final'
// - BuiltInReader
// - TessellatedMesh
// https://en.cppreference.com/w/cpp/language/final
// and why: https://devblogs.microsoft.com/oldnewthing/20200619-00/?p=103877
#include "CADMesh.hh"

G4VSolid* G4CadSystemFactory::buildSolid(GOptions* gopt, GVolume *s, map<string, G4Volume*> *g4s)
{

	string g4name = s->getG4Name();
	bool verbosity = getVerbosity(gopt, g4name);

	// check dependencies first
	if(!checkSolidDependencies(verbosity, s, g4s)) return nullptr;

	// if the g4volume doesn't exist, create one and add it to the map
	G4Volume *thisG4Volume = nullptr;

	// check if g4s already exists
	if(g4s->find(g4name) != g4s->end()) {
		thisG4Volume = (*g4s)[g4name];
		// if the solid is already built, returning it
		if (thisG4Volume->getSolid() != nullptr) return thisG4Volume->getSolid();
	} else {
		thisG4Volume = new G4Volume();
		// adding volume to the map
		(*g4s)[g4name] = thisG4Volume;
	}

	string fileName  = s->getDescription();
	G4String g4filename = fileName;
	
	auto extension = getStringVectorFromStringWithDelimiter(fileName, ".").back();


	if ( extension == "ply" ) {
		auto mesh = CADMesh::TessellatedMesh::FromPLY(g4filename);
		mesh->SetScale(CLHEP::mm);
		mesh->SetReverse(false);

		thisG4Volume->setSolid(mesh->GetSolid(), verbosity);
		return thisG4Volume->getSolid();

	} else if ( extension == "stl" ) {
		auto mesh = CADMesh::TessellatedMesh::FromSTL(g4filename);
		mesh->SetScale(CLHEP::mm);
		mesh->SetReverse(false);
		thisG4Volume->setSolid(mesh->GetSolid(), verbosity);
		return thisG4Volume->getSolid();
	}

	
	return nullptr;

}
