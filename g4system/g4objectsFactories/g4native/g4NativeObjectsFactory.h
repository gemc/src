#pragma once
/**
 * @file   g4NativeObjectsFactory.h
 * @ingroup g4system_geometry
 * @brief  Factory that builds Geant4 native primitive solids (\c G4Box, \c G4Cons, \c G4Trap, ...) from GEMC \c GVolume records.
 *
 * @details
 * This factory is responsible for solids that map directly to Geant4 CSG primitives.
 * It validates parameter counts and converts the GEMC parameter string into numeric vectors
 * used by Geant4 constructors.
 */

// c++
#include <set>            // std::set  (parameter-count tables)
#include <vector>         // std::vector
#include <unordered_map>  // std::unordered_map

// g4system
#include "g4NativeObjectsFactory.h"
#include "g4systemConventions.h"

// gemc utility helpers
#include "gutilities.h"

#include "g4objectsFactory.h"

/**
 * @class G4NativeSystemFactory
 * @ingroup g4system_geometry
 * @brief Implements solid creation for Geant4 CSG primitives and validates constructor parameter counts.
 *
 * @details
 * The method \ref G4NativeSystemFactory::buildSolid "buildSolid()" dispatches on \c s->getType()
 * and creates the corresponding Geant4 solid. The helper
 * \ref G4NativeSystemFactory::checkAndReturnParameters "checkAndReturnParameters()" validates and parses
 * the parameter list.
 */
class G4NativeSystemFactory final : public G4ObjectsFactory
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using G4ObjectsFactory::G4ObjectsFactory;

	/**
	 * @brief Human-readable name used for logging.
	 * @return \c "G4NativeSystemFactory".
	 */
	[[nodiscard]] std::string_view className() const override { return "G4NativeSystemFactory"; }

protected:
	/**
	 * @brief Create (or reuse) a native Geant4 solid based on the \c GVolume "type".
	 *
	 * @param s   GEMC volume definition containing the type and parameters.
	 * @param g4s Map holding cached \c G4Volume wrappers.
	 * @return Pointer to the created/reused solid, or \c nullptr if dependencies are missing or type unsupported.
	 *
	 * @details
	 * The method performs:
	 * - dependency checks (copy/boolean operations)
	 * - wrapper retrieval/creation
	 * - parameter parsing and validation
	 * - dispatch to the proper Geant4 constructor
	 */
	G4VSolid* buildSolid(const GVolume*                              s,
	                     std::unordered_map<std::string, G4Volume*>* g4s) override;

	/**
	 * @brief Validate the number of parameters for the given primitive and return them as numeric values.
	 *
	 * @param s Pointer to the GEMC volume record whose \c getParameters() string is to be parsed.
	 * @return A \c std::vector<double> holding the converted values in the order expected by the constructor.
	 *
	 * @details
	 * Some primitives have fixed parameter counts, while others have special rules:
	 * - polycones accept multiple constructor layouts, validated via modular arithmetic
	 * - polyhedra accept multiple constructor layouts, validated similarly
	 *
	 * When validation fails, an error is emitted through the logger.
	 */
	std::vector<double> checkAndReturnParameters(const GVolume* s);
};
