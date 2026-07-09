#pragma once
/**
 * \file   g4volume.h
 * @ingroup g4system_geometry
 * \brief  Thin wrapper that stores the Geant4 solid/logical/physical triple for one GEMC volume.
 *
 * @details
 * g4system factories progressively build Geant4 geometry in three stages:
 *   1. create a \c G4VSolid
 *   2. wrap it into a \c G4LogicalVolume (material + visualization)
 *   3. place it into the geometry tree as a \c G4VPhysicalVolume
 *
 * This class caches the three pointers so that different build steps can share the same
 * already-created objects without having to re-query Geant4 stores.
 *
 * @note
 * This wrapper does **not** own any Geant4 object. Pointer lifetimes are managed by Geant4
 * stores and by the code that registers volumes into those stores.
 */

// c++
#include <memory>

// gemc
#include <gemc/glogging/glogger.h>

// ─── Geant4 includes ───────────────────────────────────────────────
#include "G4FieldManager.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSolid.hh"

/**
 * @class G4Volume
 * @ingroup g4system_geometry
 * \brief Convenience container holding a Geant4 *solid*, *logical*, and *physical* volume.
 *
 * @details
 * A G4Volume instance is typically created and cached in a map keyed by the Geant4 volume name.
 * Each setter stores the corresponding pointer and emits a debug message through the provided logger.
 *
 * Typical usage is internal to the geometry factories:
 * - solid creation stores a \c G4VSolid*
 * - logical creation stores a \c G4LogicalVolume*
 * - physical placement stores a \c G4VPhysicalVolume*
 *
 * \note No pointer is deleted here.
 */
class G4Volume
{
public:
	/**
	 * \brief Default construct an empty wrapper (all pointers set to \c nullptr).
	 */
	G4Volume() = default;

	/**
	 * \brief Destructor (no-op).
	 * @details This class does not own the stored pointers.
	 */
	~G4Volume() = default;

	// ────── getters ────────────────────────────────────────────────

	/**
	 * \brief Return the stored \c G4VSolid pointer.
	 * \return Raw pointer, possibly \c nullptr if the solid has not been created yet.
	 */
	[[nodiscard]] G4VSolid* getSolid() const noexcept { return solidVolume; }

	/**
	 * \brief Return the stored \c G4LogicalVolume pointer.
	 * \return Raw pointer, possibly \c nullptr if the logical volume has not been created yet.
	 */
	[[nodiscard]] G4LogicalVolume* getLogical() const noexcept { return logicalVolume; }

	/**
	 * \brief Return the stored \c G4VPhysicalVolume pointer.
	 * \return Raw pointer, possibly \c nullptr if the physical volume has not been placed yet.
	 */
	[[nodiscard]] G4VPhysicalVolume* getPhysical() const noexcept { return physicalVolume; }

	/**
	 * \brief Return the volume's local frame rotation, recorded at solid-creation time.
	 *
	 * @details Used when this solid is the second operand of a boolean operation:
	 * the operation applies the operand's own rotation/position as the relative
	 * transform, following the GEMC2 `Operation:` convention.
	 */
	[[nodiscard]] const G4RotationMatrix& getSolidRotation() const noexcept { return solidRotation; }

	/// \brief Return the volume's local position, recorded at solid-creation time.
	[[nodiscard]] const G4ThreeVector& getSolidTranslation() const noexcept { return solidTranslation; }

	/**
	 * \brief Record the volume's local frame rotation and position.
	 *
	 * \param rot Frame rotation parsed from the volume definition.
	 * \param pos Position parsed from the volume definition.
	 */
	void setSolidPlacement(const G4RotationMatrix& rot, const G4ThreeVector& pos) {
		solidRotation    = rot;
		solidTranslation = pos;
	}

	// ────── setters ────────────────────────────────────────────────

	/**
	 * \brief Store a \c G4VSolid pointer and log the assignment.
	 *
	 * \param s   Solid pointer to store (may be \c nullptr).
	 * \param log Logger used for debug output.
	 *
	 * @details
	 * The call only stores the pointer; it does not validate or take ownership.
	 */
	void setSolid(G4VSolid* s, const std::shared_ptr<GLogger>& log);

	/**
	 * \brief Store a \c G4LogicalVolume pointer and log the assignment.
	 *
	 * \param l   Logical volume pointer to store (may be \c nullptr).
	 * \param log Logger used for debug output.
	 *
	 * @details
	 * The call only stores the pointer; it does not validate or take ownership.
	 */
	void setLogical(G4LogicalVolume* l, const std::shared_ptr<GLogger>& log);

	/**
	 * \brief Store a \c G4VPhysicalVolume pointer and log the assignment.
	 *
	 * \param p   Physical volume pointer to store (may be \c nullptr).
	 * \param log Logger used for debug output.
	 *
	 * @details
	 * The call only stores the pointer; it does not validate or take ownership.
	 */
	void setPhysical(G4VPhysicalVolume* p, const std::shared_ptr<GLogger>& log);

	/**
	 * \brief Attach a \c G4FieldManager to the stored logical volume, if present.
	 *
	 * \param fm                  Field manager to attach.
	 * \param forceToAllDaughters If \c true, propagate the field manager to all daughter logical volumes.
	 *
	 * @details
	 * If no logical volume has been created yet (\ref G4Volume::getLogical "getLogical()" returns \c nullptr),
	 * the method does nothing.
	 */
	void setFieldManager(G4FieldManager* fm, bool forceToAllDaughters);

private:
	// ────── data members ───────────────────────────────────────────

	/** Stored \c G4VSolid pointer (may be \c nullptr). */
	G4VSolid* solidVolume{nullptr};

	/** Local frame rotation recorded at solid-creation time (boolean operand transform). */
	G4RotationMatrix solidRotation;

	/** Local position recorded at solid-creation time (boolean operand transform). */
	G4ThreeVector solidTranslation;

	/** Stored \c G4LogicalVolume pointer (may be \c nullptr). */
	G4LogicalVolume* logicalVolume{nullptr};

	/** Stored \c G4VPhysicalVolume pointer (may be \c nullptr). */
	G4VPhysicalVolume* physicalVolume{nullptr};
};
