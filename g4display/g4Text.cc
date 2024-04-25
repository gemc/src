// g4display
#include "g4SceneProperties.h"
#include "g4Text.h"

// c++
using namespace std;

// options added to G4SceneProperties
vector<string> G4SceneProperties::addSceneTexts(GOptions *gopts) {
    vector<string> commands;


    vector<g4display::JSceneText> jstexts = g4display::getSceneTexts(gopts);

    // looking over each of the vector<json> items
    for (const auto &jstext: jstexts) {

        commands.push_back("/vis/set/textColour " + jstext.color);
        string position = to_string(jstext.x) + " " + to_string(jstext.y);
        string size = " " + to_string(jstext.size) + " ! ! ";
        commands.push_back("/vis/scene/add/text2D " + position + size + jstext.text);
        commands.push_back("/vis/set/textColour");
    }


    vector<g4display::JViewText> jvtexts = g4display::getViewTexts(gopts);

    // looking over each of the vector<json> items
    for (const auto &jvtext: jvtexts) {

        commands.push_back("/vis/set/textColour " + jvtext.color);
        string position = to_string(jvtext.x) + " " + to_string(jvtext.y) + " " + to_string(jvtext.z) + " cm ";
        string size = " " + to_string(jvtext.size) + " ! ! ";
        commands.push_back("/vis/scene/add/text " + position + size + jvtext.text);
        commands.push_back("/vis/set/textColour");
    }


    return commands;

}


namespace g4display {

    void from_json(const json &j, JSceneText &jstext);

    // scenetext
    void from_json(const json &j, JSceneText &jstext) {
        j.at("text").get_to(jstext.text);
        j.at("color").get_to(jstext.color);
        j.at("x").get_to(jstext.x);
        j.at("y").get_to(jstext.y);
        j.at("size").get_to(jstext.size);
    }

    // method to return a vector of JSceneText from a structured option
    vector<JSceneText> getSceneTexts(GOptions *gopts) {

        vector<JSceneText> st;

        auto jsceneTexts = gopts->getStructuredOptionAssignedValues("scenetext");

        // looking over each of the vector<json> items
        for (const auto &jsceneText: jsceneTexts) {
            st.push_back(jsceneText.get<JSceneText>());
        }

        return st;
    }


    // viewtext
    void from_json(const json &j, JViewText &jvtext) {
        j.at("text").get_to(jvtext.text);
        j.at("color").get_to(jvtext.color);
        j.at("x").get_to(jvtext.x);
        j.at("y").get_to(jvtext.y);
        j.at("z").get_to(jvtext.z);
        j.at("size").get_to(jvtext.size);
    }

    // method to return a vector of JSceneText from a structured option
    vector<JViewText> getViewTexts(GOptions *gopts) {

        vector<JViewText> vt;

        auto jviewTexts = gopts->getStructuredOptionAssignedValues("viewtext");

        // looking over each of the vector<json> items
        for (const auto &jviewText: jviewTexts) {
            vt.push_back(jviewText.get<JViewText>());
        }

        return vt;
    }


    // returns array of options definitions
    vector<GOption> addSceneTextsOptions() {

        vector<GOption> goptions;

        // scenetext
        // cumulative: can use -add
        json jsonSceneTextTag = {
                {GNAME, "text"},
                {GDESC, "scene text (mandatory)."},
                {GDFLT, NODFLT}
        };

        json jsonSceneTextColorTag = {
                {GNAME, "color"},
                {GDESC, "scene text color (optional). Possible values are color names such as green, red, etc. Default is white."},
                {GDFLT, "black"}
        };

        json jsonSceneTextXPosTag = {
                {GNAME, "x"},
                {GDESC, "scene text x position (optional). Possible values: between -1 and 1. Default is 0."},
                {GDFLT, 0}
        };

        json jsonSceneTextYPosTag = {
                {GNAME, "y"},
                {GDESC, "scene text y position (optional). Possible values: between -1 and 1. Default is 0."},
                {GDFLT, 0}
        };

        json jsonSceneTextSizeTag = {
                {GNAME, "size"},
                {GDESC, "scene text size (optional). Default is 24"},
                {GDFLT, 24}
        };


        json jsonSceneTextOption = {
                jsonSceneTextTag,
                jsonSceneTextColorTag,
                jsonSceneTextXPosTag,
                jsonSceneTextYPosTag,
                jsonSceneTextSizeTag
        };

        vector<string> help;
        help.push_back("Adds a scene text. The text does not move with the detector. ");
        help.push_back("");
        help.push_back("Example: +scenetext={text: \"lhc experiment\", color: \"green\", x: 0.5, y: 0.5}");

        // the last argument refers to "cumulative"
        goptions.push_back(GOption("scenetext", "adds text to the scene", jsonSceneTextOption, help, true));

        return goptions;
    }


    // returns array of options definitions
    vector<GOption> addViewTextsOptions() {

        vector<GOption> goptions;

        // scenetext
        // cumulative: can use -add
        json jsonViewTextTag = {
                {GNAME, "text"},
                {GDESC, "view text (mandatory)."},
                {GDFLT, NODFLT}
        };

        json jsonViewTextColorTag = {
                {GNAME, "color"},
                {GDESC, "view text color (optional). Possible values are color names such as green, red, etc. Default is white."},
                {GDFLT, "black"}
        };

        json jsonViewTextXPosTag = {
                {GNAME, "x"},
                {GDESC, "view text x position in cm. Default is 0."},
                {GDFLT, 0}
        };

        json jsonViewTextYPosTag = {
                {GNAME, "y"},
                {GDESC, "view text y position in cm. Default is 0."},
                {GDFLT, 0}
        };

        json jsonViewTextZPosTag = {
                {GNAME, "z"},
                {GDESC, "view text z position in cm. Default is 0."},
                {GDFLT, 0}
        };

        json jsonViewTextSizeTag = {
                {GNAME, "size"},
                {GDESC, "view text size (optional). Default is 24"},
                {GDFLT, 24}
        };


        json jsonviewTextOption = {
                jsonViewTextTag,
                jsonViewTextColorTag,
                jsonViewTextXPosTag,
                jsonViewTextYPosTag,
                jsonViewTextZPosTag,
                jsonViewTextSizeTag
        };

        vector<string> help;
        help.push_back("Adds a view text. ");
        help.push_back("");
        help.push_back("Example: +viewtext={text: \"lhc experiment\", color: \"green\", x: 5, y: 5, z: 30}");

        // the last argument refers to "cumulative"
        goptions.push_back(GOption("viewtext", "adds text to the view", jsonviewTextOption, help, true));

        return goptions;
    }
}
