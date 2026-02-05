#pragma once
/**
 * @file   cadSystemFactory.h
 * @ingroup g4system_geometry
 * @brief  Factory that converts CAD files (PLY / STL) into Geant4 tessellated solids via CADMesh.
 *
 * @details
 * This factory expects that the input volume description provides a CAD file path
 * (typically via the \c GVolume "description" field). Supported formats are:
 * - \c .ply
 * - \c .stl
 *
 * The implementation uses CADMesh (single-header library) and enables an Assimp reader backend
 * in the implementation translation unit.
 *
 * Requirements:
 * - \c CADMesh.hh must be available on the include path
 * - Assimp must be available if the Assimp reader backend is enabled
 */

// c++
#include <unordered_map>
#include <string>

// g4system
#include "g4objectsFactory.h"

/**
 * @class G4CadSystemFactory
 * @ingroup g4system_geometry
 * @brief Builds a tessellated solid from CAD files using CADMesh.
 *
 * @details
 * The factory implements \ref G4CadSystemFactory::buildSolid "buildSolid()" to produce a \c G4VSolid.
 * Logical and physical construction are provided by the base class and therefore follow the same
 * material lookup, visualization attribute, and placement rules used by other factories.
 */
class G4CadSystemFactory final : public G4ObjectsFactory
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using G4ObjectsFactory::G4ObjectsFactory;

	/**
	 * @brief Factory label used in logs.
	 * @return \c "G4CadSystemFactory".
	 */
	[[nodiscard]] std::string_view className() const override { return "G4CadSystemFactory"; }

protected:
	/**
	 * @brief Create (or fetch) a tessellated solid from a CAD file.
	 *
	 * @param s   GEMC volume definition. The file path is read from \c s->getDescription().
	 * @param g4s Map holding cached \c G4Volume wrappers.
	 * @return Pointer to the created solid, or \c nullptr if dependencies are missing or format unsupported.
	 *
	 * @details
	 * The method performs:
	 * - dependency checks (copy/boolean operands) through the shared base logic
	 * - wrapper retrieval/creation for caching
	 * - extension-based dispatch to the CADMesh reader
	 *
	 * Recognized extensions are:
	 * - \c ply
	 * - \c stl
	 *
	 * Any other extension results in a warning and \c nullptr.
	 */
	G4VSolid* buildSolid(const GVolume*                              s,
	                     std::unordered_map<std::string, G4Volume*>* g4s) override;
};
