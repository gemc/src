#pragma once
/**
 * @file   g4volume.h
 * @ingroup Geometry
 * @brief  Thin RAII wrapper that stores the Geant4 solid/logical/physical triple.
 *
 * A `gemc::G4Volume` does **not** own the Geant4 objects; it merely keeps the
 * pointers so they can be shared between geometry‑building steps.  These helpers
 * are inexpensive, inline, and never throw.
 */

#include <memory>

#include "glogger.h"

// ─── Geant4 includes ───────────────────────────────────────────────
#include "G4FieldManager.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VSolid.hh"


/**
 * @class G4Volume
 * @brief Convenience container holding a Geant4 *solid*, *logical*, and
 *        *physical* volume associated with a single GEMC `GVolume`.
 *
 * The setters simply cache the given pointer and emit a DEBUG log entry.
 * No pointer is deleted here—the lifetime is managed by Geant4’s own stores.
 */
class G4Volume {
public:
    /** Default‑construct an empty wrapper (all pointers set to `nullptr`). */
    G4Volume() = default;

    /** Destructor – no‑op because we do not own the pointers. */
    ~G4Volume() = default;

    // ────── getters ────────────────────────────────────────────────
    /**
     * @brief Retrieve the underlying `G4VSolid*`.
     * @return Raw pointer which may be `nullptr`.
     */
    [[nodiscard]] G4VSolid* getSolid() const noexcept { return solidVolume; }

    /**
     * @brief Retrieve the underlying `G4LogicalVolume*`.
     * @return Raw pointer which may be `nullptr`.
     */
    [[nodiscard]] G4LogicalVolume* getLogical() const noexcept { return logicalVolume; }

    /**
     * @brief Retrieve the underlying `G4VPhysicalVolume*`.
     * @return Raw pointer which may be `nullptr`.
     */
    [[nodiscard]] G4VPhysicalVolume* getPhysical() const noexcept { return physicalVolume; }

    // ────── setters ────────────────────────────────────────────────
    /**
     * @brief Store a `G4VSolid*` and log the assignment.
     * @param s   Pointer to the Geant4 solid.
     * @param log Logger used for debug output.
     */
    void setSolid(G4VSolid* s, const std::shared_ptr<GLogger>& log);

    /**
     * @brief Store a `G4LogicalVolume*` and log the assignment.
     * @param l   Pointer to the Geant4 logical volume.
     * @param log Logger used for debug output.
     */
    void setLogical(G4LogicalVolume* l, const std::shared_ptr<GLogger>& log);

    /**
     * @brief Store a `G4VPhysicalVolume*` and log the assignment.
     * @param p   Pointer to the Geant4 physical volume.
     * @param log Logger used for debug output.
     */
    void setPhysical(G4VPhysicalVolume* p, const std::shared_ptr<GLogger>& log);

    /**
     * @brief Attach a `G4FieldManager` to the logical volume, if present.
     * @param fm                    Field manager to apply.
     * @param forceToAllDaughters   Propagate to daughters if `true`.
     *
     * Does nothing if `logicalVolume` is `nullptr`.
     */
    void setFieldManager(G4FieldManager* fm, bool forceToAllDaughters);

private:
    // ────── data members ───────────────────────────────────────────
    /** Pointer to the Geant4 solid (maybe `nullptr`). */
    G4VSolid* solidVolume {nullptr};

    /** Pointer to the Geant4 logical volume (may be `nullptr`). */
    G4LogicalVolume* logicalVolume {nullptr};

    /** Pointer to the Geant4 physical volume (may be `nullptr`). */
    G4VPhysicalVolume* physicalVolume {nullptr};
};

