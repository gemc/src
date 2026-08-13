/**
 * \file   buildSolid.cc
 * @ingroup g4system_geometry
 * \brief  Implementation of the CAD-mesh solid builder for G4CadSystemFactory.
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

namespace {
	bool numeric_value(const std::string& value, double& result) {
		try {
			size_t parsed = 0;
			result = std::stod(value, &parsed);
			return parsed == value.size();
		}
		catch (const std::exception&) { return false; }
	}
}

G4VSolid* G4CadSystemFactory::buildSolid(const GVolume* s,
                                         std::unordered_map<std::string,
                                                            G4Volume*>* g4s) {
	std::string g4name = s->getG4Name();

	// Dependency check: solids can require other solids (copy/boolean operations).
	if (!checkSolidDependencies(s, g4s)) return nullptr;

	// Locate or allocate the wrapper used to cache solid/logical/physical pointers.
	auto thisG4Volume = getOrCreateG4Volume(g4name, g4s);
	if (thisG4Volume->getSolid() != nullptr) return thisG4Volume->getSolid();

	// If this is a copy of another volume, reuse the source mesh solid instead of loading a mesh:
	// the copy carries no mesh file of its own (e.g. the LTCC frame plates placed in every sector).
	std::string copyOf = s->getCopyOf();
	if (copyOf != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) {
		auto sourceName     = s->getSystem() + "/" + copyOf;
		auto sourceG4Volume = getOrCreateG4Volume(sourceName, g4s);
		if (sourceG4Volume->getSolid() != nullptr) return sourceG4Volume->getSolid();
	}

	// Current rows store "<mesh path>, <scale>" in parameters. For compatibility, an old row may
	// store only its numeric scale there and its mesh path in description.
	std::string fileName = s->getDescription();
	double      scale    = 1.0;
	const auto  cadParameters =
		gutilities::getStringVectorFromStringWithDelimiter(s->getParameters(), ",");
	if (cadParameters.size() > 1) {
		fileName = cadParameters[0];
		if (!numeric_value(cadParameters[1], scale)) {
			log->warning("G4CadSystemFactory: volume <", g4name,
			             "> has a non-numeric scale <", cadParameters[1], ">; using 1.0");
			scale = 1.0;
		}
	}
	else if (cadParameters.size() == 1 && cadParameters[0] != "NULL") {
		double legacyScale = 1.0;
		if (numeric_value(cadParameters[0], legacyScale)) { scale = legacyScale; }
		else { fileName = cadParameters[0]; }
	}

	G4String    g4filename = fileName;

	// File extension (last token after '.').
	std::string extension =
		gutilities::getStringVectorFromStringWithDelimiter(fileName, ".").back();

	// PLY / STL via CADMesh & Assimp reader.
	if (extension == "ply" || extension == "stl") {
		auto mesh = CADMesh::TessellatedMesh::From(g4filename,
		                                           CADMesh::File::ASSIMP());

		// The CAD file is interpreted in millimetres (times the per-volume scale) to match
		// typical detector CAD conventions.
		mesh->SetScale(CLHEP::mm * scale);

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
