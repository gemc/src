/**
 * @file   cadSystemFactory.cc
 * @ingroup Geometry
 * @brief  Implementation of the CAD‑mesh solid builder.
 */

// gemc
#include "gutilities.h"

// g4system
#include "cadSystemFactory.h"

// ───────────────── CADMesh single‑header library ────────────────
// https://github.com/christopherpoole/CADMesh
//
// *Modifications applied to the vendor header:*
//   - Marked the following classes `final`
//       • BuiltInReader
//       • TessellatedMesh
//   - Reason: https://devblogs.microsoft.com/oldnewthing/20200619-00/?p=103877
//
// *Optional Dependencies*
//   Define `USE_CADMESH_ASSIMP_READER` **before** including `CADMesh.hh`
//   to enable Assimp‑based file loading.
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! SET THIS BEFORE INCLUDING CADMESH.HH TO USE THE ASSIMP READER
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define USE_CADMESH_ASSIMP_READER
// If you want Assimp to be the default reader, uncomment:
// #define CADMESH_DEFAULT_READER ASSIMP

#include "CADMesh.hh"

// Geant4 units
#include "CLHEP/Units/SystemOfUnits.h"

G4VSolid* G4CadSystemFactory::buildSolid(const GVolume* s,
                                         std::unordered_map<std::string,
                                                            G4Volume*>* g4s) {
	std::string g4name = s->getG4Name();

	/*────────── dependency check ──────────*/
	if (!checkSolidDependencies(s, g4s)) return nullptr;

	/*────────── locate or allocate G4Volume ──────────*/
	auto thisG4Volume = getOrCreateG4Volume(g4name, g4s);
	if (thisG4Volume->getSolid() != nullptr) return thisG4Volume->getSolid();

	/*────────── CAD file handling ──────────*/
	std::string fileName   = s->getDescription(); // full file path from DB
	G4String    g4filename = fileName;

	// file extension
	std::string extension =
		gutilities::getStringVectorFromStringWithDelimiter(fileName, ".").back();

	/*──── PLY / STL via CADMesh & Assimp reader ────*/
	if (extension == "ply" || extension == "stl") {
		auto mesh = CADMesh::TessellatedMesh::From(g4filename,
		                                           CADMesh::File::ASSIMP());

		mesh->SetScale(CLHEP::mm); // keep geometry in Geant4 length units
		mesh->SetReverse(false);   // vertex winding not flipped

		thisG4Volume->setSolid(mesh->GetSolid(), log);
		return thisG4Volume->getSolid();
	}

	/*──── unsupported extension ────*/
	log->warning("G4CadSystemFactory: file <", fileName,
	             "> has unsupported extension <", extension, ">");
	return nullptr;
}
