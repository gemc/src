#pragma once

// geant4
#include "G4VUserDetectorConstruction.hh"

// gemc
#include "gStateMessage.h"
#include "gsd.h"
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "gworld.h"
#include "g4world.h"
#include "gmagneto.h"

#include <map>
#include <string>
#include <vector>

using std::map;
using std::string;
using std::vector;

/**
 * GDetectorConstruction builds the Geant4 geometry based on GEMC world definitions.
 * It also constructs sensitive detectors, fields, and digitization plugins.
 */
class GDetectorConstruction : public G4VUserDetectorConstruction, public GStateMessage {
public:
	/**
	 * Constructor.
	 * @param gopts Pointer to GEMC options.
	 * @param gDDGlobal Pointer to the global map of GDynamicDigitization objects.
	 */
	GDetectorConstruction(GOptions *gopts, map<string, GDynamicDigitization *> *gDDGlobal);

	/// Destructor.
	virtual ~GDetectorConstruction();

public:
	// Geant4 virtual methods.
	virtual G4VPhysicalVolume *Construct();
	virtual void ConstructSDandField();

	// Accessor methods.
	bool is_empty() const { return g4world->is_empty(); }
	int get_number_of_volumes() const { return gworld->get_number_of_volumes(); }
	int get_number_of_g4_volumes() const { return g4world->get_number_of_volumes(); }

	/**
	 * Reloads the geometry using a new vector of GSystem objects.
	 * @param gs New vector of GSystem objects.
	 */
	void reload_geometry(vector<GSystem> gs);

private:
	GOptions *gopt = nullptr;
	GWorld *gworld = nullptr;
	G4World *g4world = nullptr;

	// The global map of dynamic digitization plugins (shared with event dispenser).
	map<string, GDynamicDigitization *> *gDynamicDigitizationMapGlobalInstance;

	// Contains fields and field managers.
	static G4ThreadLocal GMagneto *gmagneto;

	// Loads digitization plugins after sensitive detectors have been set up.
	void loadDigitizationPlugins();

	// Collection of GSystem objects. If empty, geometry is built from options.
	vector<GSystem> gsystems;
};


