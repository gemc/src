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
 * GDetectorConstruction builds the Geant4 geometry based on GEMC world definitions.
 * It also constructs sensitive detectors, fields, and digitization plugins.
 */
class GDetectorConstruction : public GBase<GDetectorConstruction>, public G4VUserDetectorConstruction {
public:
	/**
	 * Constructor.
	 * @param gopts Pointer to GEMC options.
	 */
	explicit GDetectorConstruction(std::shared_ptr<GOptions> gopts);

public:
	// Geant4 virtual methods.
	G4VPhysicalVolume* Construct() override;
	void               ConstructSDandField() override;

	// Accessor methods.
	[[nodiscard]] bool is_empty() const { return g4world->is_empty(); }

	/**
	 * Reloads the geometry using a new vector of GSystem objects.
	 * @param sl new vector of gsystem pointers.
	 */
	void reload_geometry(SystemList sl);

	std::shared_ptr<GDynamicDigitization> get_digitization_routines_for_sdname(const std::string &sd_name) const {
		return digitization_routines_map->at(sd_name);
	}

	std::shared_ptr<gdynamicdigitization::dRoutinesMap> get_digitization_routines_map() const {
		return digitization_routines_map;
	}

	[[nodiscard]] std::unordered_map<std::string, G4Volume*> get_g4volumes_map() const {
		if (g4world == nullptr) {
			log->error(ERR_G4VOLUMEBUILDFAILED, "No g4world ");
		}
		return g4world->get_g4volumes_map();
	}

private:
	std::shared_ptr<GOptions> gopt;    // need options inside Constructs() methods
	std::shared_ptr<GWorld>   gworld;
	std::shared_ptr<G4World>  g4world;

	// digitization map for all sensitive detectors
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;

	// Contains fields and field managers.
	// this needs to be a raw pointer as its ownerwhip is passed to G4
	static G4ThreadLocal GMagneto* gmagneto;

	// Loads digitization plugins after sensitive detectors have been set up.
	void loadDigitizationPlugins();

	// Collection of GSystem objects. If empty, geometry is built from options.
	SystemList gsystems;
};

