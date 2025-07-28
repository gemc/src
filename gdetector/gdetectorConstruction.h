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
	virtual G4VPhysicalVolume* Construct() override;
	virtual void               ConstructSDandField() override;

	// Accessor methods.
	bool is_empty() const { return g4world->is_empty(); }

	/**
	 * Reloads the geometry using a new vector of GSystem objects.
	 * @param gs New vector of GSystem objects.
	 */
	void reload_geometry(SystemList sl);

private:
	std::shared_ptr<GOptions> gopt;    // need options inside Constructs() methods
	std::shared_ptr<GLogger>  log;
	std::shared_ptr<GWorld>   gworld;
	std::shared_ptr<G4World>  g4world;
	std::unordered_map<std::string,  std::shared_ptr<GSensitiveDetector>> sensitiveDetectorsMap; // keeping GSensitiveDetector on scope until geometry is destroyed


	// thread local - digitization for this sensitive detector
	std::shared_ptr<GDynamicDigitization> digitization_routine;

	// Contains fields and field managers.
	static G4ThreadLocal std::unique_ptr<GMagneto> gmagneto;

	// Loads digitization plugins after sensitive detectors have been set up.
	void loadDigitizationPlugins();

	// Collection of GSystem objects. If empty, geometry is built from options.
	SystemList gsystems;
};
