// g4display 
#include "g4SceneProperties.h"
#include "g4display_options.h"
#include "g4Text.h"

using namespace g4display;

// gemc
#include "gutilities.h"

// c++
using namespace std;

// geant4
#include "G4UImanager.hh"


G4SceneProperties::G4SceneProperties(GOptions* gopts): log(new GLogger(gopts, G4DISPLAY_LOGGER)) {
	bool gui = gopts->getSwitch("gui");

	G4UImanager* g4uim = G4UImanager::GetUIpointer();

	// projecting options onto G4View and G4Camera structs
	G4View g4view = getG4View(gopts);
	G4Camera g4camera = getG4Camera(gopts);
	G4Dawn g4dawn = getG4Dawn(gopts);
	bool use_dawn = gopts->getSwitch("useDawn");

	vector<string> commands;

	commands.emplace_back("/vis/scene/create gemc");
	commands.emplace_back("/run/initialize");

	if (use_dawn) {
		commands.emplace_back("/vis/open DAWNFILE");
		commands.emplace_back("/vis/geometry/set/visibility World 0 false");
		commands.emplace_back("/vis/viewer/set/style surface");
	}

	if (gui) { commands.emplace_back("/vis/open " + g4view.viewer + " " + g4view.dimension + g4view.position); }

	// Disable auto refresh and quieten vis messages whilst scene is established:
	commands.emplace_back("/vis/viewer/set/autoRefresh false");
	commands.emplace_back("/vis/drawVolume");

	// scene texts
	for (const string& c : addSceneTexts(gopts)) { commands.emplace_back(c); }

	double toDegrees = 180 / 3.1415;
	double thetaValue = gutilities::getG4Number(g4camera.theta) * toDegrees;
	double phiValue = gutilities::getG4Number(g4camera.phi) * toDegrees;

	commands.emplace_back("/vis/viewer/set/viewpointThetaPhi " + to_string(thetaValue) + " " + to_string(phiValue));
	commands.emplace_back("/vis/viewer/set/lineSegmentsPerCircle " + to_string(g4view.segsPerCircle));
	commands.emplace_back("/vis/viewer/set/autoRefresh true");

	if (use_dawn) { commands.emplace_back("/vis/viewer/flush"); }

	if (g4uim) {
		for (auto& c : commands) {
			log->info(1, "Executing UIManager command \"" + c + "\"");
			g4uim->ApplyCommand(c.c_str());
		}
	}
	else { cout << " No UIManager found. " << endl; }
}


vector<string> G4SceneProperties::addSceneTexts(GOptions* gopts) {
	vector<string> commands;

	vector<g4display::G4SceneText> text_to_add = g4display::getSceneTexts(gopts);

	// looking over each of the vector<json> items
	for (const auto& text : text_to_add) {
		commands.emplace_back("/vis/set/textColour " + text.color);
		string position = to_string(text.x) + " " + to_string(text.y);
		string size = " " + to_string(text.size) + " ! ! ";
		if (text.z != GNOT_SPECIFIED_SCENE_TEXT_Z) {
			position += " " + to_string(text.z);
			commands.emplace_back(string("/vis/scene/add/text2D ").append(position).append(size).append(text.text));
		}
		else { commands.emplace_back(string("/vis/scene/add/text ").append(position).append(size).append(text.text)); }

		commands.emplace_back("/vis/set/textColour");
	}


	return commands;
}
