#pragma once
/**
 * @file   cadSystemFactory.h
 * @ingroup Geometry
 * @brief  Factory that converts a CAD file (PLY / STL) into a Geant4
 *         tessellated solid via the **CADMesh** single‑header library.
 *
 * Requirements:
 * * `CADMesh.hh` must be available on the include path.
 * * Define `USE_CADMESH_ASSIMP_READER` *before* including the header
 *   if you need the Assimp I/O backend (enabled below).
 */

#include <unordered_map>
#include <string>

#include "g4objectsFactory.h"

/**
 * @class G4CadSystemFactory
 * @brief Builds a `G4TessellatedSolid` from PLY/STL CAD files.
 *
 * The factory relies on *CADMesh* — a header‑only helper that parses CAD
 * formats and produces Geant4 tessellated meshes.
 */
class G4CadSystemFactory final : public G4ObjectsFactory
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using G4ObjectsFactory::G4ObjectsFactory;

	/**
	 * @copydoc G4ObjectsFactory::className
	 * @return `"G4CadSystemFactory"`.
	 */
	[[nodiscard]] std::string_view className() const override
	{ return "G4CadSystemFactory"; }

protected:
	/**
	 * @brief Create (or fetch) a tessellated solid from a CAD file.
	 *
	 * Recognised extensions: **`.ply`** and **`.stl`**.  All other types
	 * return `nullptr`.
	 */
	G4VSolid* buildSolid(const GVolume*                                   s,
						 std::unordered_map<std::string, G4Volume*>*      g4s) override;
};
