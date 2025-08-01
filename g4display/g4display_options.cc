// g4display
#include "g4display_options.h"
#include "g4displayConventions.h"
#include "g4Text.h"

using namespace std;

// namespace to define options
namespace g4display {
// full list from /vis/list
// TODO: lets get that programmatically
// ASCIITree ATree DAWNFILE G4HepRepFile HepRepFile RayTracer VRML2FILE gMocrenFile OpenGLImmediateQt OGLIQt OGLI OpenGLStoredQt OGLSQt OGL OGLS
// vector<string> AVAILABLEG4VIEWERS = {
// 	"TOOLSSG_QT_GLES",
// 	"OpenGLImmediateQt",
// 	"OGLIQt",
// 	"OGLI",
// 	"OpenGLStoredQt",
// 	"OGLSQt",
// 	"OGL",
// 	"OGLS"
// };

// read g4view option and return G4View struct
G4View getG4View(const std::shared_ptr<GOptions>& gopts) {
	// projecting it onto G4View structure
	G4View g4view;
	g4view.driver        = gopts->getOptionMapInNode("g4view", "driver").as<string>();;
	g4view.dimension     = gopts->getOptionMapInNode("g4view", "dimension").as<string>();
	g4view.position      = gopts->getOptionMapInNode("g4view", "position").as<string>();
	g4view.segsPerCircle = gopts->getOptionMapInNode("g4view", "segsPerCircle").as<int>();

	return g4view;
}

// read the g4camera option and return a G4Camera struct
G4Camera getG4Camera(const std::shared_ptr<GOptions>& gopts) {
	G4Camera gcamera;
	gcamera.phi   = gopts->getOptionMapInNode("g4camera", "phi").as<string>();
	gcamera.theta = gopts->getOptionMapInNode("g4camera", "theta").as<string>();

	return gcamera;
}

// read the dawn options and return a G4Dawn struct
G4Dawn getG4Dawn(const std::shared_ptr<GOptions>& gopts) {
	G4Dawn gdawn;

	auto phi   = gopts->getOptionMapInNode("dawn", "phi").as<string>();
	auto theta = gopts->getOptionMapInNode("dawn", "theta").as<string>();

	if (phi == "null") phi = goptions::NODFLT;
	if (theta == "null") theta = goptions::NODFLT;

	gdawn.phi   = phi;
	gdawn.theta = theta;

	return gdawn;
}


// returns array of options definitions
GOptions defineOptions() {

	GOptions goptions(G4DISPLAY_LOGGER);
	string   help;

	// g4display
	// string VIEWERCHOICES = "g4 viewer. Available choices:\n\n";
	// for (const auto& c : AVAILABLEG4VIEWERS) { VIEWERCHOICES += "\t\t\t\t- " + c + "\n"; }

	vector<GVariable> g4view = {
		{"driver", string(GDEFAULTVIEWERDRIVER), "Geant4 vis driver"},
		{"dimension", string(GDEFAULTVIEWERSIZE), "g4 viewer dimension"},
		{"position", string(GDEFAULTVIEWERPOS), "g4 viewer position"},
		{"segsPerCircle", GDEFAULTVSEGPERCIRCLE, "Number of segments per circle"}

	};

	help = "Defines the geant4 viewer properties:  \n ";
	help += " - screen dimensions  \n ";
	help += " - screen position  \n ";
	help += " - resolution in terms of segments per circle  \n ";
	help +=
		" Example: -g4view={viewer: \"OGL\", dimension: \"1100x800\", position: \"+200+100\", segsPerCircle: 100}  \n \n";
	help += "-g4view=\"[{dimensions: 1200x1000}]\"\n";
	goptions.defineOption("g4view", "Defines the geant4 viewer properties", g4view, help);


	// g4camera
	vector<GVariable> g4camera = {
		{"phi", "0*deg", "geant4 camera phi"},
		{"theta", "0*deg", "geant4 camera theta"}
	};

	help = "Defines the geant4 camera view point  \n \n ";
	help += "Example: -g4camera=\"[{phi: 20*deg, theta: 15*deg}]\"  \n ";
	goptions.defineOption("g4camera", "Defines the geant4 camera view point", g4camera, help);


	// dawn
	help = "Defines the dawn camera view point and take a dawn screenshot \n \n ";
	help += "Example: -dawn=\"[{phi: 20*deg, theta: 15*deg}]\"  \n ";
	vector<GVariable> dawn = {
		{"phi", 30, "dawn phi"},
		{"theta", 30, "dawn theta"}
	};
	goptions.defineOption("dawn", "Defines the dawn view point", dawn, help);
	goptions.defineSwitch("useDawn", "Take a dawn screenshot");

	// scenetext
	goptions.addGOptions(addSceneTextsOptions());

	return goptions;
}
}
