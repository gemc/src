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
            for (auto map_item = g4t_item.begin(); map_item != g4t_item.end(); ++map_item) {
                if (map_item->first.as<string>() == "text") {
                    st_item.text = map_item->second.as<string>();
                } else if (map_item->first.as<string>() == "color") {
                    st_item.color = map_item->second.as<string>();
                } else if (map_item->first.as<string>() == "x") {
                    st_item.x = map_item->second.as<float>();
                } else if (map_item->first.as<string>() == "y") {
                    st_item.y = map_item->second.as<float>();
                } else if (map_item->first.as<string>() == "z") {
                    st_item.z = map_item->second.as<float>();
                } else if (map_item->first.as<string>() == "size") {
                    st_item.size = map_item->second.as<int>();
                }
            }
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
                {"size",  24,                      "size of the text"},
        };
        goptions.defineOption("g4text", "Insert texts in the current scene", g4text, help);

        return goptions;
    }


}
