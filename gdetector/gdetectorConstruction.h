#pragma once

// geant4
#include "G4VUserDetectorConstruction.hh"

// gemc
#include "goptions.h"
#include "g4world.h"
#include "gmagneto.h"
#include "gsd.h"
#include "gbase.h"

// c++
#include <vector>

/**
 * @file gdetectorConstruction.h
 * @brief Defines the \ref GDetectorConstruction class, the Geant4 detector-construction
 *        entry point for the gdetector module.
 *
 * This header is the authoritative Doxygen documentation for the \ref GDetectorConstruction
 * API. Implementation files provide only short non-Doxygen summaries (see rule 7).
 *
 * @ingroup gdetector_module
 */

/**
 * @class GDetectorConstruction
 * @brief Builds and (optionally) reloads the Geant4 geometry from GEMC world definitions.
 *
 * \ref GDetectorConstruction is the module’s adapter between GEMC geometry/detector
 * descriptions (GWorld/GSystem/GVolume) and Geant4 runtime objects (solids, logical
 * volumes, physical volumes, sensitive detectors, and field managers).
 *
 * Responsibilities:
 * - Geometry construction:
 *   - Builds a GEMC world (GWorld) either from options or from an externally supplied
 *     list of systems.
 *   - Builds a Geant4 world (G4World) from that GEMC world.
 * - Sensitive detector construction:
 *   - Creates and registers GSensitiveDetector instances by digitization name.
 *   - Registers GTouchable metadata for each sensitive volume.
 * - Electromagnetic field setup:
 *   - Creates (thread-local) field infrastructure through GMagneto when volumes
 *     declare an EM field.
 *   - Installs per-volume field managers into the Geant4 logical volumes.
 * - Digitization routine setup:
 *   - Loads digitization plugins and associates them with each sensitive detector.
 *
 * Threading notes:
 * - Geant4 supports multi-threading with thread-local resources. The field container
 *   pointer (GMagneto) is stored as \c G4ThreadLocal to keep field state thread-safe.
 *
 * Ownership notes:
 * - The Geant4 runtime owns some resources after registration. For example, the field
 *   infrastructure is maintained via a raw pointer because ownership is transferred
 *   to Geant4 managers.
 */
class GDetectorConstruction : public GBase<GDetectorConstruction>, public G4VUserDetectorConstruction {
public:
	/**
	 * @brief Constructs a detector builder configured by the provided options.
	 *
	 * The options object is retained for later use during \c Construct() and
	 * \c ConstructSDandField().
	 *
	 * @param gopts Shared options instance (must remain valid for the lifetime of
	 *              this detector construction object).
	 */
	explicit GDetectorConstruction(std::shared_ptr<GOptions> gopts);

public:
	/**
	 * @brief Geant4 geometry construction hook.
	 *
	 * Called by Geant4 when it needs the detector geometry. This method:
	 * - Clears any previously built geometry stores (when reloading).
	 * - Constructs a GWorld either from options or from the previously provided
	 *   systems list (see \ref GDetectorConstruction::reload_geometry "reload_geometry()").
	 * - Builds a G4World from the GWorld.
	 *
	 * @return The Geant4 physical world volume.
	 */
	G4VPhysicalVolume* Construct() override;

	/**
	 * @brief Geant4 SD/field construction hook.
	 *
	 * Called by Geant4 to install sensitive detectors and EM fields after the
	 * geometry is built. This method:
	 * - Creates GSensitiveDetector instances on-demand by digitization name.
	 * - Assigns each sensitive detector to the relevant Geant4 logical volumes.
	 * - Creates field managers when volumes declare EM fields and installs them.
	 * - Loads digitization routines and binds them to the sensitive detectors.
	 */
	void ConstructSDandField() override;

	/**
	 * @brief Returns whether the currently built geometry is empty.
	 *
	 * This delegates to the underlying G4World instance, which tracks whether
	 * volumes were successfully built.
	 *
	 * @return True if no geometry is currently present, false otherwise.
	 */
	[[nodiscard]] bool is_empty() const { return g4world->is_empty(); }

	/**
	 * @brief Reloads the geometry using a new list of GSystem objects.
	 *
	 * This method updates the internal systems list used by \c Construct().
	 * If a Geant4 run manager exists, it triggers a re-definition of the world
	 * volume and re-installs sensitive detectors and fields.
	 *
	 * @param sl New list of systems to build from. If empty, the previous system
	 *           list is kept (useful for tests or when only forcing a rebuild).
	 */
	void reload_geometry(SystemList sl);

	/**
	 * @brief Returns the digitization routine for a given sensitive detector name.
	 *
	 * The returned object is the per-sensitive-detector dynamic digitization plugin.
	 *
	 * @param sd_name Sensitive detector name (digitization routine key).
	 * @return Shared pointer to the digitization routine associated with @p sd_name.
	 *
	 * @warning This uses \c at() on the underlying map and will throw if @p sd_name
	 *          is not present. Callers should ensure the detector name exists in
	 *          the geometry sensitive-detector list.
	 */
	std::shared_ptr<GDynamicDigitization> get_digitization_routines_for_sdname(const std::string &sd_name) const {
		return digitization_routines_map->at(sd_name);
	}

	/**
	 * @brief Returns the full map of digitization routines.
	 *
	 * This exposes the internal mapping between sensitive detector names and their
	 * corresponding digitization plugins.
	 *
	 * @return Shared pointer to the digitization routines map.
	 */
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> get_digitization_routines_map() const {
		return digitization_routines_map;
	}

	/**
	 * @brief Returns a map of built Geant4 volumes indexed by their GEMC/G4World names.
	 *
	 * This method reports an error if the Geant4 world has not been constructed yet.
	 *
	 * @return A copy of the map from volume name to G4Volume pointer.
	 */
	[[nodiscard]] std::unordered_map<std::string, G4Volume*> get_g4volumes_map() const {
		if (g4world == nullptr) {
			log->error(ERR_G4VOLUMEBUILDFAILED, "No g4world ");
		}
		return g4world->get_g4volumes_map();
	}

private:
	/**
	 * @brief Cached options used during construction and SD/field setup.
	 *
	 * This must remain valid throughout the lifetime of \ref GDetectorConstruction.
	 */
	std::shared_ptr<GOptions> gopt;    // need options inside Constructs() methods

	/**
	 * @brief GEMC world representation: systems, volumes, materials, and detector metadata.
	 *
	 * Recreated whenever geometry is rebuilt (initial build or reload).
	 */
	std::shared_ptr<GWorld>   gworld;

	/**
	 * @brief Geant4 world representation built from \ref GDetectorConstruction::gworld.
	 *
	 * Recreated whenever geometry is rebuilt (initial build or reload).
	 */
	std::shared_ptr<G4World>  g4world;

	/**
	 * @brief Digitization routines for all sensitive detectors in the current geometry.
	 *
	 * The map key is the sensitive detector name (digitization routine name), and the
	 * value is the dynamic digitization plugin instance.
	 */
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;

	/**
	 * @brief Thread-local container for EM field objects and field managers.
	 *
	 * This is a raw pointer because ownership is passed into Geant4 infrastructure.
	 * It is thread-local to match Geant4 multi-threading patterns.
	 */
	static G4ThreadLocal GMagneto* gmagneto;

	/**
	 * @brief Loads digitization plugins after sensitive detectors have been set up.
	 *
	 * This method populates \ref GDetectorConstruction::digitization_routines_map and
	 * configures each routine (logger, readout specs). It is intentionally private and
	 * not cross-referenced from documentation (rule 6).
	 */
	void loadDigitizationPlugins();

	/**
	 * @brief Collection of GSystem objects used when rebuilding geometry.
	 *
	 * If empty, geometry is built entirely from options (the typical "full GEMC run"
	 * behavior). If populated, geometry is rebuilt using these systems (reload path).
	 */
	SystemList gsystems;
};
