#pragma once
/**
 * @file   cadSystemFactory.h
 * @ingroup Geometry
 * @brief  Factory that converts CAD files (PLY / STL) into Geant4 tessellated solids via CADMesh.
 *
 * @details
 * This factory expects that the input volume description provides a CAD file path
 * (typically via the \c GVolume "description" field). Supported formats are:
 * - \c .ply
 * - \c .stl
 *
 * The implementation uses CADMesh (single-header library) and enables the Assimp reader backend.
 *
 * Requirements:
 * - \c CADMesh.hh must be available on the include path.
 * - The Assimp backend is enabled in the implementation translation unit.
 */

// c++
#include <unordered_map>
#include <string>

// g4system
#include "g4objectsFactory.h"

/**
 * @class G4CadSystemFactory
 * @ingroup Geometry
 * @brief Builds a tessellated solid from CAD files using CADMesh.
 *
 * @details
 * The factory implements \ref G4ObjectsFactory::buildSolid "buildSolid()" and relies on the base class
 * for logical/physical creation. The solid is cached in the \c G4Volume wrapper map.
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
	 * Recognized extensions are:
	 * - \c ply
	 * - \c stl
	 *
	 * Any other extension results in a warning and \c nullptr.
	 */
	G4VSolid* buildSolid(const GVolume*                              s,
	                     std::unordered_map<std::string, G4Volume*>* g4s) override;
};
