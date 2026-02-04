/*!
 * \file g4SceneProperties.cc
 * \brief Implementation of \ref G4SceneProperties.
 *
 * Doxygen documentation for public behavior is authoritative in g4SceneProperties.h (see rule 7).
 */

// g4display
#include "g4SceneProperties.h"
#include "g4Text.h"

// gemc
#include "gutilities.h"

// geant4
#include "G4UImanager.hh"

std::vector<std::string> G4SceneProperties::scene_commands(const std::shared_ptr<GOptions>& gopts) {
	std::vector<std::string> cmds;

	bool gui      = gopts->getSwitch("gui");
	bool use_dawn = gopts->getSwitch("useDawn");

	// Project options onto simple structs for downstream use.
	auto g4view   = g4display::getG4View(gopts);
	auto g4camera = g4display::getG4Camera(gopts);

	std::vector<std::string> commands;

	// Create a named scene. Caller is expected to apply these commands to the Geant4 UI manager.
	cmds.emplace_back("/vis/scene/create gemc");

	if (use_dawn) {
		// DAWNFILE workflow: open the DAWN viewer and adjust a minimal set of scene properties.
		cmds.emplace_back("/vis/open DAWNFILE");
		cmds.emplace_back("/vis/geometry/set/visibility World 0 false");
		cmds.emplace_back("/vis/viewer/set/style surface");
	}

	if (gui) {
		// Open the configured viewer driver with window geometry settings.
		cmds.emplace_back("/vis/open " + g4view.driver + " " + g4view.dimension + g4view.position);

		// Scene texts: generate and append per configured g4text option.
		for (const std::string& c : addSceneTexts(gopts)) { commands.emplace_back(c); }

		// Convert configured camera angles to degrees for the Geant4 viewer command.
		double toDegrees  = 180 / 3.1415;
		double thetaValue = gutilities::getG4Number(g4camera.theta) * toDegrees;
		double phiValue   = gutilities::getG4Number(g4camera.phi) * toDegrees;

		// Disable auto refresh and quieten vis messages whilst scene and trajectories are established.
		cmds.emplace_back("/vis/viewer/set/autoRefresh false");
		cmds.emplace_back(
			"/vis/viewer/set/viewpointThetaPhi " + std::to_string(thetaValue) + " " + std::to_string(phiValue));
		cmds.emplace_back("/vis/viewer/set/lineSegmentsPerCircle " + std::to_string(g4view.segsPerCircle));
		cmds.emplace_back("/vis/viewer/set/autoRefresh true");
	}

	return cmds;
}

std::vector<std::string> G4SceneProperties::addSceneTexts(const std::shared_ptr<GOptions>& gopts) {
	std::vector<std::string> commands;

	std::vector<g4display::G4SceneText> text_to_add = g4display::getSceneTexts(gopts);

	// Map each configured text item into Geant4 text commands.
	for (const auto& text : text_to_add) {
		commands.emplace_back("/vis/set/textColour " + text.color);

		std::string position = std::to_string(text.x) + " " + std::to_string(text.y);
		std::string size     = " " + std::to_string(text.size) + " ! ! ";

		if (text.z != GNOT_SPECIFIED_SCENE_TEXT_Z) {
			// Z specified: treat as 2D text positioned in 3D.
			position += " " + std::to_string(text.z);
			commands.emplace_back(
				std::string("/vis/scene/add/text2D ").append(position).append(size).append(text.text));
		}
		else {
			// Z not specified: treat as normal scene text.
			commands.emplace_back(std::string("/vis/scene/add/text ").append(position).append(size).append(text.text));
		}

		// Restore default text color.
		commands.emplace_back("/vis/set/textColour");
	}

	return commands;
}
