/*!
 * \file g4SceneProperties.cc
 * \brief Implementation of the G4SceneProperties class for GEANT4 scene visualization.
 *
 * This file contains the implementation of the G4SceneProperties class which is responsible
 * for setting up visualization options and commands for GEANT4 scenes using the provided global options.
 */

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

/*!
 * \brief Constructs a G4SceneProperties object.
 *
 * This constructor initializes the scene properties based on the provided global options.
 * It sets up visualization commands including scene creation, viewer configuration, and text annotations.
 * These commands are then executed using the GEANT4 UI manager.
 *
 * \param gopts Pointer to the global options object.
 */
G4SceneProperties::G4SceneProperties(GOptions* gopts): log(std::make_shared<GLogger>(gopts, G4DISPLAY_LOGGER, "G4SceneProperties")) {

	log->debug(CONSTRUCTOR, "G4SceneProperties");

	bool gui = gopts->getSwitch("gui");

    auto g4uim = G4UImanager::GetUIpointer();

    // Projecting options onto G4View and G4Camera structs
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

    if (gui) {
        commands.emplace_back("/vis/open " + g4view.driver + " " + g4view.dimension + g4view.position);
    }

    // Disable auto refresh and quieten vis messages whilst scene is established:
    commands.emplace_back("/vis/viewer/set/autoRefresh false");
    commands.emplace_back("/vis/drawVolume");

    // Scene texts
    for (const string& c : addSceneTexts(gopts)) {
        commands.emplace_back(c);
    }

    double toDegrees = 180 / 3.1415;
    double thetaValue = gutilities::getG4Number(g4camera.theta) * toDegrees;
    double phiValue = gutilities::getG4Number(g4camera.phi) * toDegrees;

    commands.emplace_back("/vis/viewer/set/viewpointThetaPhi " + to_string(thetaValue) + " " + to_string(phiValue));
    commands.emplace_back("/vis/viewer/set/lineSegmentsPerCircle " + to_string(g4view.segsPerCircle));
    commands.emplace_back("/vis/viewer/set/autoRefresh true");

    if (use_dawn) {
        commands.emplace_back("/vis/viewer/flush");
    }

    if (g4uim) {
        for (auto& c : commands) {
            log->info(1, "Executing UIManager command \"" + c + "\"");
            g4uim->ApplyCommand(c.c_str());
        }
    }
    else {
        cout << " No UIManager found. " << endl;
    }
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
vector<string> G4SceneProperties::addSceneTexts(GOptions* gopts) {
    vector<string> commands;

    vector<g4display::G4SceneText> text_to_add = g4display::getSceneTexts(gopts);

    // Iterate over each text object to generate the corresponding visualization commands
    for (const auto& text : text_to_add) {
        commands.emplace_back("/vis/set/textColour " + text.color);
        string position = to_string(text.x) + " " + to_string(text.y);
        string size = " " + to_string(text.size) + " ! ! ";
        if (text.z != GNOT_SPECIFIED_SCENE_TEXT_Z) {
            position += " " + to_string(text.z);
            commands.emplace_back(string("/vis/scene/add/text2D ").append(position).append(size).append(text.text));
        }
        else {
            commands.emplace_back(string("/vis/scene/add/text ").append(position).append(size).append(text.text));
        }

        commands.emplace_back("/vis/set/textColour");
    }

    return commands;
}
