/*!
 * \file g4SceneProperties.cc
 * \brief Implementation of the G4SceneProperties class for GEANT4 scene visualization.
 *
 * This file contains the implementation of the G4SceneProperties class, which is responsible
 * for setting up visualization options and commands for GEANT4 scenes using the provided global options.
 */

// g4display
#include "g4SceneProperties.h"
#include "g4display_options.h"
#include "g4Text.h"

// gemc
#include "gutilities.h"

// geant4
#include "G4UImanager.hh"

/*!
 * \brief Constructs a G4SceneProperties object.
 *
 * This constructor initializes the scene properties based on the provided global options.
 * It sets up visualization commands including scene creation, viewer configuration, and text annotations.
 * These commands are then executed using the GEANT4 UI manager.
 *
 * \param gopts Pointer to the global options object.
 */
G4SceneProperties::G4SceneProperties(const std::shared_ptr<GOptions>& gopts): log(std::make_shared<GLogger>(gopts, G4DISPLAY_LOGGER, "G4SceneProperties")) {
	log->debug(CONSTRUCTOR, "G4SceneProperties");
}

std::vector<std::string> G4SceneProperties::scene_commands(const std::shared_ptr<GOptions>& gopts) {
	std::vector<std::string> cmds;

	bool gui      = gopts->getSwitch("gui");
	bool use_dawn = gopts->getSwitch("useDawn");

	// Projecting options onto G4View and G4Camera structs
	auto g4view   = g4display::getG4View(gopts);
	auto g4camera = g4display::getG4Camera(gopts);
	// auto g4dawn   = g4display::getG4Dawn(gopts);

	std::vector<std::string> commands;

	cmds.emplace_back("/vis/scene/create gemc");

	if (use_dawn) {
		cmds.emplace_back("/vis/open DAWNFILE");
		cmds.emplace_back("/vis/geometry/set/visibility World 0 false");
		cmds.emplace_back("/vis/viewer/set/style surface");
	}

	if (gui) {
		cmds.emplace_back("/vis/open " + g4view.driver + " " + g4view.dimension + g4view.position);

		// Scene texts
		for (const std::string& c : addSceneTexts(gopts)) { commands.emplace_back(c); }

		double toDegrees  = 180 / 3.1415;
		double thetaValue = gutilities::getG4Number(g4camera.theta) * toDegrees;
		double phiValue   = gutilities::getG4Number(g4camera.phi) * toDegrees;

		// Disable auto refresh and quieten vis messages whilst scene and trajectories are established:
		cmds.emplace_back("/vis/viewer/set/autoRefresh false");
		cmds.emplace_back("/vis/viewer/set/viewpointThetaPhi " + std::to_string(thetaValue) + " " + std::to_string(phiValue));
		cmds.emplace_back("/vis/viewer/set/lineSegmentsPerCircle " + std::to_string(g4view.segsPerCircle));
		cmds.emplace_back("/vis/viewer/set/autoRefresh true");
	}


	return cmds;
}


/*!
 * \brief Generates visualization commands for adding text annotations to the scene.
 *
 * This function retrieves a list of scene text objects from the global options and constructs
 * the appropriate commands to add text annotations to the scene. It sets the text color, position,
 * and size based on the parameters provided. Depending on whether a Z-coordinate is specified,
 * the function chooses between 2D and regular text commands.
 *
 * \param gopts Pointer to the global options object.
 * \return A vector of strings containing the visualization commands for scene texts.
 */
std::vector<std::string> G4SceneProperties::addSceneTexts(const std::shared_ptr<GOptions>& gopts) {
	std::vector<std::string> commands;

	std::vector<g4display::G4SceneText> text_to_add = g4display::getSceneTexts(gopts);

	// Iterate over each text object to generate the corresponding visualization commands
	for (const auto& text : text_to_add) {
		commands.emplace_back("/vis/set/textColour " + text.color);
		std::string position = std::to_string(text.x) + " " + std::to_string(text.y);
		std::string size     = " " + std::to_string(text.size) + " ! ! ";
		if (text.z != GNOT_SPECIFIED_SCENE_TEXT_Z) {
			position += " " + std::to_string(text.z);
			commands.emplace_back(std::string("/vis/scene/add/text2D ").append(position).append(size).append(text.text));
		}
		else { commands.emplace_back(std::string("/vis/scene/add/text ").append(position).append(size).append(text.text)); }

		commands.emplace_back("/vis/set/textColour");
	}

	return commands;
}
