// g4display_options.cc
//
// Implementation of g4display option projection and option schema definition.
// Doxygen documentation for public behavior is authoritative in g4display_options.h (see rule 7).

#include "g4display_options.h"
#include "g4displayConventions.h"
#include "g4Text.h"

namespace g4display {
// Read g4view option and return a projected G4View struct.
G4View getG4View(const std::shared_ptr<GOptions>& gopts) {
	G4View g4view;

	// Project the YAML-like option node values into strongly-typed fields.
	g4view.driver        = gopts->getOptionMapInNode("g4view", "driver").as<std::string>();
	g4view.dimension     = gopts->getOptionMapInNode("g4view", "dimension").as<std::string>();
	g4view.position      = gopts->getOptionMapInNode("g4view", "position").as<std::string>();
	g4view.segsPerCircle = gopts->getOptionMapInNode("g4view", "segsPerCircle").as<int>();

	return g4view;
}

// Read the g4camera option and return a G4Camera struct.
G4Camera getG4Camera(const std::shared_ptr<GOptions>& gopts) {
	G4Camera gcamera;

	// Camera angles are stored as strings (often with units), and can be parsed later.
	gcamera.phi   = gopts->getOptionMapInNode("g4camera", "phi").as<std::string>();
	gcamera.theta = gopts->getOptionMapInNode("g4camera", "theta").as<std::string>();

	return gcamera;
}

// Read the g4light option and return a G4Light struct.
G4Light getG4Light(const std::shared_ptr<GOptions>& gopts) {
	G4Light glight;

	// Light angles are stored as strings (often with units), and can be parsed later.
	glight.phi   = gopts->getOptionMapInNode("g4light", "phi").as<std::string>();
	glight.theta = gopts->getOptionMapInNode("g4light", "theta").as<std::string>();

	return glight;
}

// Read the dawn options and return a G4Dawn struct.
G4Dawn getG4Dawn(const std::shared_ptr<GOptions>& gopts) {
	G4Dawn gdawn;

	auto phi   = gopts->getOptionMapInNode("dawn", "phi").as<std::string>();
	auto theta = gopts->getOptionMapInNode("dawn", "theta").as<std::string>();

	// Normalize explicit "null" (string) to the “not defined” sentinel used by options.
	if (phi == "null") phi = goptions::NODFLT;
	if (theta == "null") theta = goptions::NODFLT;

	gdawn.phi   = phi;
	gdawn.theta = theta;

	return gdawn;
}

// Define and return the option set for the g4display module.
GOptions defineOptions() {
	GOptions goptions(G4DISPLAY_LOGGER);

	// The module also defines options for g4scene helpers (same executable context).
	goptions += GOptions(G4SCENE_LOGGER);

	std::string help;

	// g4view
	std::vector<GVariable> g4view = {
		{"driver", std::string(GDEFAULTVIEWERDRIVER), "Geant4 visualization driver. Use TOOLSSG_OFFSCREEN in batch mode. "},
		{"dimension", std::string(GDEFAULTVIEWERSIZE), "Geant4 viewer dimension"},
		{"position", std::string(GDEFAULTVIEWERPOS), "Geant4 viewer position"},
		{"segsPerCircle", GDEFAULTVSEGPERCIRCLE, "Number of segments per circle"}
	};

	help = "Defines the Geant4 viewer properties:  \n ";
	help += " - screen dimensions  \n ";
	help += " - screen position  \n ";
	help += " - resolution in terms of segments per circle  \n \n ";
	help += " Examples: \n \n ";
	help += " -g4view=\"[{dimension: 1200x1000}]\"\n";
	help += " -g4view=\"[{driver: OGL, dimension: 1100x800, position: +200+100, segsPerCircle: 100}]\" \n";
	help += " -g4view=\"[{driver: TOOLSSG_OFFSCREEN, segsPerCircle: 200}]\" takes a screenshot at the end of each run \n";

	goptions.defineOption("g4view", "Defines the geant4 viewer properties", g4view, help);

	// g4camera
	std::vector<GVariable> g4camera = {
		{"phi", "0*deg", "Geant4 camera phi"},
		{"theta", "0*deg", "Geant4 camera theta"}
	};

	help = "Defines the geant4 camera view point  \n \n ";
	help += "Example: -g4camera=\"[{phi: 20*deg, theta: 15*deg}]\"  \n ";

	goptions.defineOption("g4camera", "Defines the geant4 camera view point", g4camera, help);

	// g4light
	std::vector<GVariable> g4light = {
		{"phi", "0*deg", "Geant4 light source phi"},
		{"theta", "0*deg", "Geant4 light source theta"}
	};

	help = "Defines the geant4 light source direction  \n \n ";
	help += "Example: -g4light=\"[{phi: 20*deg, theta: 15*deg}]\"  \n ";

	goptions.defineOption("g4light", "Defines the geant4 light source direction", g4light, help);

	// dawn
	help = "Defines the dawn camera view point and take a dawn screenshot \n \n ";
	help += "Example: -dawn=\"[{phi: 20*deg, theta: 15*deg}]\"  \n ";

	std::vector<GVariable> dawn = {
		{"phi", 30, "dawn phi"},
		{"theta", 30, "dawn theta"}
	};

	goptions.defineOption("dawn", "Defines the dawn view point", dawn, help);
	goptions.defineSwitch("useDawn", "Take a dawn screenshot");

	// scenetext
	goptions.addGOptions(addSceneTextsOptions());

	return goptions;
}
} // namespace g4display
