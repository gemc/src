#pragma once
/**
 * @file   g4NativeObjectsFactory.h
 * @ingroup Geometry
 * @brief  Factory that builds Geant4 *native primitive* solids
 *         (`G4Box`, `G4Cons`, `G4Trap`, …) from GEMC `GVolume` records.
 *
 * Concrete detector systems can register this factory in a `GManager`
 * so that run‑time construction happens via plug‑ins.
 */

#include <set>            // std::set  (parameter‑count tables)
#include <vector>         // std::vector
#include <unordered_map>  // std::unordered_map

// g4system
#include "g4NativeObjectsFactory.h"      // self include‑guard pattern

// gemc utility helpers
#include "gutilities.h"

#include "g4objectsFactory.h"

/**
 * @class G4NativeSystemFactory
 * @brief Implements @c buildSolid for the Geant4 CSG primitives and validates
 *        constructor parameter counts via @c checkAndReturnParameters.
 */
class G4NativeSystemFactory final : public G4ObjectsFactory {
public:
	/** Default constructor/destructor. */
	G4NativeSystemFactory()           = default;
	~G4NativeSystemFactory() override = default;

	/**
	 * @brief Human‑readable name used by the base class for logging.
	 * @return `"G4NativeSystemFactory"`.
	 */
	[[nodiscard]] std::string_view className() const override { return "G4NativeSystemFactory"; }

protected:
	/**
	 * @brief Main dispatcher that creates a solid based on `s->getType()`.
	 * @copydetails G4ObjectsFactory::buildSolid
	 */
	G4VSolid* buildSolid(const GVolume*                              s,
	                     std::unordered_map<std::string, G4Volume*>* g4s) override;

	/**
	 * @brief Validate the *number* of parameters against the Geant4 constructor
	 *        requirements and return them as a numeric vector.
	 *
	 * @param s Pointer to the GEMC volume record whose `getParameters()` string
	 *          is to be parsed.
	 * @return A `std::vector<double>` holding the converted values.
	 *
	 * If the parameter count is invalid, the function logs an error and
	 * terminates (via the logger’s error helper).  Special rules apply for
	 * polycones, polyhedra, and twisted solids, as documented in the code.
	 *
	 * @note This is **protected** because subclasses could reuse the checker
	 *       if they add support for additional primitives.
	 */
	std::vector<double> checkAndReturnParameters(const GVolume* s);
};
