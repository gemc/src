// g4display 
#include "g4SceneProperties.h"
#include "g4displayOptions.h"

using namespace g4display;

// gemc
#include "gutilities.h"

using namespace gutilities;

// c++
using namespace std;

// geant4
#include "G4UImanager.hh"


G4SceneProperties::G4SceneProperties(GOptions *gopts) {
    // PRAGMA TODO: add resolution option, hits options, here?
    // PRAGMA TOOO: if run/beamOn is given (e.g. command line options) then the label needs to be updated!

    // notice gui and verbosity are not set in g4display but in main()
    bool gui = gopts->getSwitch("gui");
    int verbosity = gopts->getVerbosityFor("g4display");

    G4UImanager *g4uim = G4UImanager::GetUIpointer();

    // projecting options onto G4View and G4Camera structs
    G4View g4view = getG4View(gopts);
    G4Camera g4camera = getG4Camera(gopts);
    G4Dawn g4dawn = getG4Dawn(gopts);
    bool use_dawn = g4dawn.phi != goptions::NODFLT;

    vector <string> commands;

    commands.push_back("/vis/scene/create gemc");
    commands.push_back("/run/initialize");

    if (use_dawn) {
        commands.push_back("/vis/open DAWNFILE");
        commands.push_back("/vis/geometry/set/visibility World 0 false");
        commands.push_back("/vis/viewer/set/style surface");
    }

    if (gui) {
        commands.push_back("/vis/open " + g4view.viewer + " " + g4view.dimension + g4view.position);
    }

    // Disable auto refresh and quieten vis messages whilst scene is established:
    commands.push_back("/vis/viewer/set/autoRefresh false");
    commands.push_back("/vis/drawVolume");

    // scene texts
    for (string c: addSceneTexts(gopts)) {
        commands.push_back(c);
    }

    double toDegrees = 180 / 3.1415;
    double thetaValue = getG4Number(g4camera.theta) * toDegrees;
    double phiValue = getG4Number(g4camera.phi) * toDegrees;

    commands.push_back("/vis/viewer/set/viewpointThetaPhi " + to_string(thetaValue) + " " + to_string(phiValue));
    commands.push_back("/vis/viewer/set/lineSegmentsPerCircle " + to_string(g4view.segsPerCircle));
    commands.push_back("/vis/viewer/set/autoRefresh true");

    if (use_dawn) {
        commands.push_back("/vis/viewer/flush");
    }

    if (g4uim) {
        for (auto &c: commands) {
            if (verbosity > GVERBOSITY_SUMMARY) {
                cout << TPOINTITEM << "Executing UIManager command \"" << c << "\"" << endl;
            }
            g4uim->ApplyCommand(c.c_str());
        }

    } else {
        cout << " No UIManager found. " << endl;
    }

}


vector <string> G4SceneProperties::addSceneTexts(GOptions *gopts) {

    vector <string> commands;

    vector <g4display::G4SceneText> text_to_add = g4display::getSceneTexts(gopts);

    // looking over each of the vector<json> items
    for (const auto &text: text_to_add) {

        commands.push_back("/vis/set/textColour " + text.color);
        string position = to_string(text.x) + " " + to_string(text.y);
        string size = " " + to_string(text.size) + " ! ! ";
        if (text.z != GNOT_SPECIFIED_SCENE_TEXT_Z) {
            position += " " + to_string(text.z);
            commands.push_back("/vis/scene/add/text2D " + position + size + text.text);
        } else {
            commands.push_back("/vis/scene/add/text " + position + size + text.text);
        }

        commands.push_back("/vis/set/textColour");
    }


    return commands;

}
