#pragma once

// geant4
#include "G4VUserDetectorConstruction.hh"

// gemc
#include "goptions.h"
#include "g4world.h"
#include "gmagneto.h"
#include "gsd.h"

// c++
#include <vector>

/**
 * GDetectorConstruction builds the Geant4 geometry based on GEMC world definitions.
 * It also constructs sensitive detectors, fields, and digitization plugins.
 */
class GDetectorConstruction : public G4VUserDetectorConstruction {
public:
	/**
	 * Constructor.
	 * @param gopts Pointer to GEMC options.
	 */
	explicit GDetectorConstruction(std::shared_ptr<GOptions> gopts);

	/// Destructor.
	~GDetectorConstruction() override;

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
	void reload_geometry(SystemList sl = {nullptr});

	std::shared_ptr<GDynamicDigitization> get_digitization_routines_for_sdname(const std::string &sd_name) const {
		return digitization_routines_map->at(sd_name);
	}

	std::shared_ptr<gdynamicdigitization::dRoutinesMap> get_digitization_routines_map() const {
		return digitization_routines_map;
	}

private:
	std::shared_ptr<GOptions> gopt;    // need options inside Constructs() methods
	std::shared_ptr<GLogger>  log;
	std::shared_ptr<GWorld>   gworld;
	std::shared_ptr<G4World>  g4world;

	// GSensitiveDetector must be a raw pointer because geant4 takes ownership
	std::unordered_map<std::string,  GSensitiveDetector*> sensitiveDetectorsMap; // keeping GSensitiveDetector on scope until geometry is destroyed


	// digitization map for all sensitive detectors
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;

	// Contains fields and field managers.
	static G4ThreadLocal std::unique_ptr<GMagneto> gmagneto;

	// Loads digitization plugins after sensitive detectors have been set up.
	void loadDigitizationPlugins();

	// Collection of GSystem objects. If empty, geometry is built from options.
	SystemList gsystems;
};

