// g4display
#include "g4SceneProperties.h"
#include "g4Text.h"

// c++
using namespace std;


namespace g4display {

    // method to return a vector of G4SceneText from a structured option
    vector <G4SceneText> getSceneTexts(GOptions *gopts) {

        vector <G4SceneText> st;

        auto g4t_node = gopts->get_option_node("g4text");

        for (auto g4t_item: g4t_node) {
            G4SceneText st_item;

            st_item.text = gopts->get_variable_in_option<string>(g4t_item, "text", goptions::NODFLT);
            st_item.color = gopts->get_variable_in_option<string>(g4t_item, "color", "black");
            st_item.x = gopts->get_variable_in_option<float>(g4t_item, "x", 0);
            st_item.y = gopts->get_variable_in_option<float>(g4t_item, "y", 0);
            st_item.z = gopts->get_variable_in_option<float>(g4t_item, "z", GNOT_SPECIFIED_SCENE_TEXT_Z);
            st_item.size = gopts->get_variable_in_option<double>(g4t_item, "size", 24.0);
            st.push_back(st_item);
        }

        return st;
    }


    // returns array of options definitions
    GOptions addSceneTextsOptions() {

        GOptions goptions;
        string help;

        // g4text
        help = "If the z coordinate is specified, the text is considered 2D. \n \n";
        help = "Example to add two texts: \n \n";
        help += "-g4text=\"[{text: hello, x: -100}, {text: there, x: 100}]\"\n";
        vector <GVariable> g4text = {
                {"text",  goptions::NODFLT,        "string with the text to be displayed"},
                {"color", "black",                 "color of the text"},
                {"x",     0,                       "x position of the text"},
                {"y",     0,                       "y position of the text"},
                {"z", GNOT_SPECIFIED_SCENE_TEXT_Z, "z position of the text"},
                {"size",  24.0,                    "size of the text"},
        };
        goptions.defineOption("g4text", "Insert texts in the current scene", g4text, help);

        return goptions;
    }

}
