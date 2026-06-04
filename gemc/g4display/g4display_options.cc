// g4display_options.cc
//
// Implementation of g4display option projection and option schema definition.
// Doxygen documentation for public behavior is authoritative in g4display_options.h (see rule 7).

#include "g4display_options.h"
#include "g4displayConventions.h"
#include "g4Text.h"

namespace g4display {
namespace {
YAML::Node getG4ViewValue(const std::shared_ptr<GOptions>& gopts, const std::string& key) {
	auto node = gopts->getOptionNode("g4view");
	if (node.IsMap() && node[key]) {
		return node[key];
	}
	return gopts->getOptionMapInNode("g4view", key);
}

template<typename T>
T getOptionValueOrDefault(const std::shared_ptr<GOptions>& gopts,
                          const std::string& option,
                          const std::string& key,
                          const T& defaultValue) {
	auto node = gopts->getOptionNode(option);
	if (node.IsMap()) {
		return gopts->get_variable_in_option<T>(node, key, defaultValue);
	}
	return gopts->getOptionMapInNode(option, key).as<T>();
}
}

// Read g4view option and return a projected G4View struct.
G4View getG4View(const std::shared_ptr<GOptions>& gopts) {
	G4View g4view;

	// Project the YAML-like option node values into strongly-typed fields.
	g4view.driver        = getG4ViewValue(gopts, "driver").as<std::string>();
	g4view.dimension     = getG4ViewValue(gopts, "dimension").as<std::string>();
	g4view.position      = getG4ViewValue(gopts, "position").as<std::string>();
	g4view.segsPerCircle = getG4ViewValue(gopts, "segsPerCircle").as<int>();
	g4view.background    = getG4ViewValue(gopts, "background").as<std::string>();
	g4view.cloudPoints   = getG4ViewValue(gopts, "cloudPoints").as<int>();

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

G4Decorations getG4Decorations(const std::shared_ptr<GOptions>& gopts) {
	G4Decorations decorations;

	decorations.scale          = getOptionValueOrDefault<bool>(gopts, "g4decoration", "scale", false);
	decorations.scaleLength    = getOptionValueOrDefault<double>(gopts, "g4decoration", "scaleLength", 10.0);
	decorations.scaleUnit      = getOptionValueOrDefault<std::string>(gopts, "g4decoration", "scaleUnit", "mm");
	decorations.scaleDirection = getOptionValueOrDefault<std::string>(gopts, "g4decoration", "scaleDirection", "z");
	decorations.scaleColor     = getOptionValueOrDefault<std::string>(gopts, "g4decoration", "scaleColor", "0.9 0.9 0.9");
	decorations.axes           = getOptionValueOrDefault<bool>(gopts, "g4decoration", "axes", false);
	decorations.eventID        = getOptionValueOrDefault<bool>(gopts, "g4decoration", "eventID", false);
	decorations.date           = getOptionValueOrDefault<bool>(gopts, "g4decoration", "date", false);
	decorations.logo2D         = getOptionValueOrDefault<bool>(gopts, "g4decoration", "logo2D", false);
	decorations.logo           = getOptionValueOrDefault<bool>(gopts, "g4decoration", "logo", false);
	decorations.frame          = getOptionValueOrDefault<bool>(gopts, "g4decoration", "frame", false);
	decorations.frameColor     = getOptionValueOrDefault<std::string>(gopts, "g4decoration", "frameColor", "red");
	decorations.frameLineWidth = getOptionValueOrDefault<double>(gopts, "g4decoration", "frameLineWidth", 2.0);

	return decorations;
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
		{"segsPerCircle", GDEFAULTVSEGPERCIRCLE, "Number of segments per circle"},
		{"background", "0.05 0.05 0.26", "Geant4 viewer background color as '<red> <green> <blue>'"},
		{"cloudPoints", 1000, "Number of points used for cloud volume rendering"}
	};

	help = "Defines the Geant4 viewer properties:  \n ";
	help += " - screen dimensions  \n ";
	help += " - screen position  \n ";
	help += " - resolution in terms of segments per circle  \n \n ";
	help += " - viewer background color as '<red> <green> <blue>'  \n ";
	help += " - number of cloud points for cloud volume rendering  \n \n ";
	help += " Examples: \n \n ";
	help += " -g4view=\"[{dimension: 1200x1000}]\"\n";
	help += " -g4view=\"[{driver: OGL, dimension: 1100x800, position: +200+100, segsPerCircle: 100, background: 0.05 0.05 0.26}]\" \n";
	help += " -g4view=\"[{driver: TOOLSSG_OFFSCREEN, segsPerCircle: 200, cloudPoints: 3000}]\" takes a screenshot at the end of each run \n";

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

	// g4decoration
	std::vector<GVariable> g4decoration = {
		{"scale", false, "add a simple scale line"},
		{"scaleLength", 10.0, "scale length"},
		{"scaleUnit", "mm", "scale length unit"},
		{"scaleDirection", "z", "scale direction: x, y, or z"},
		{"scaleColor", "0.9 0.9 0.9", "scale color as 'r g b' or a named color"},
		{"axes", false, "add simple XYZ axes"},
		{"eventID", false, "add event ID text at end of event"},
		{"date", false, "add a date stamp"},
		{"logo2D", false, "add the 2D Geant4 logo"},
		{"logo", false, "add the 3D Geant4 logo"},
		{"frame", false, "add a frame around the view"},
		{"frameColor", "red", "frame color"},
		{"frameLineWidth", 2.0, "frame line width"}
	};

	help = "Adds optional Geant4 scene decorations. \n \n";
	help += "Example: -g4decoration=\"[{scale: true, axes: true, eventID: true, date: true, logo2D: true, logo: true, frame: true}]\" \n";

	goptions.defineOption("g4decoration", "Adds optional Geant4 scene decorations", g4decoration, help);

	// scenetext
	goptions.addGOptions(addSceneTextsOptions());

	return goptions;
}
} // namespace g4display
