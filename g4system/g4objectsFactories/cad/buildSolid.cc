/**
 * @file   buildSolid.cc
 * @ingroup g4system_geometry
 * @brief  Implementation of the CAD-mesh solid builder for G4CadSystemFactory.
 */

// gemc
#include "gutilities.h"

// g4system
#include "cadSystemFactory.h"

// ───────────────── CADMesh single-header library ────────────────
// https://github.com/christopherpoole/CADMesh
//
// *Modifications applied to the vendor header:*
//   - Marked the following classes `final`
//       • BuiltInReader
//       • TessellatedMesh
//   - Reason: https://devblogs.microsoft.com/oldnewthing/20200619-00/?p=103877
//
// *Optional Dependencies*
// Define `USE_CADMESH_ASSIMP_READER` **before** including `CADMesh.hh`
// to enable Assimp-based file loading.
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

	// Dependency check: solids can require other solids (copy/boolean operations).
	if (!checkSolidDependencies(s, g4s)) return nullptr;

	// Locate or allocate the wrapper used to cache solid/logical/physical pointers.
	auto thisG4Volume = getOrCreateG4Volume(g4name, g4s);
	if (thisG4Volume->getSolid() != nullptr) return thisG4Volume->getSolid();

	// CAD file handling:
	// - file path is stored in the volume "description" field
	// - extension determines which CADMesh reader path is used
	std::string fileName   = s->getDescription(); // full file path from DB
	G4String    g4filename = fileName;

	// File extension (last token after '.').
	std::string extension =
		gutilities::getStringVectorFromStringWithDelimiter(fileName, ".").back();

	// PLY / STL via CADMesh & Assimp reader.
	if (extension == "ply" || extension == "stl") {
		auto mesh = CADMesh::TessellatedMesh::From(g4filename,
		                                           CADMesh::File::ASSIMP());

		// The CAD file is interpreted in millimeters to match typical detector CAD conventions.
		mesh->SetScale(CLHEP::mm);

		// Do not flip vertex winding unless the CAD source requires it.
		mesh->SetReverse(false);

		thisG4Volume->setSolid(mesh->GetSolid(), log);
		return thisG4Volume->getSolid();
	}

	// Unsupported extension: return nullptr so the caller can decide whether to treat it as fatal.
	log->warning("G4CadSystemFactory: file <", fileName,
	             "> has unsupported extension <", extension, ">");
	return nullptr;
}
