// g4Text.cc
//
// Implementation of g4display scene-text option parsing.
// Doxygen documentation is authoritative in g4Text.h (see rule 7).

#include "g4Text.h"

// c++
using namespace std;

namespace g4display {
// Return a vector of G4SceneText parsed from the "g4text" structured option node.
vector<G4SceneText> getSceneTexts(const std::shared_ptr<GOptions>& gopts) {
	vector<G4SceneText> st;

	auto g4t_node = gopts->getOptionNode("g4text");

	// Each node entry becomes one text annotation.
	for (auto g4t_item : g4t_node) {
		G4SceneText st_item;

		st_item.text  = gopts->get_variable_in_option<string>(g4t_item, "text", goptions::NODFLT);
		if (st_item.text == goptions::NODFLT || st_item.text.empty()) { continue; }

		st_item.color = gopts->get_variable_in_option<string>(g4t_item, "color", "black");
		st_item.kind  = gopts->get_variable_in_option<string>(g4t_item, "kind", "2D");
		st_item.layout = gopts->get_variable_in_option<string>(g4t_item, "layout", "");
		st_item.x     = gopts->get_variable_in_option<double>(g4t_item, "x", 0);
		st_item.y     = gopts->get_variable_in_option<double>(g4t_item, "y", 0);
		st_item.z     = gopts->get_variable_in_option<double>(g4t_item, "z", GNOT_SPECIFIED_SCENE_TEXT_Z);
		st_item.unit  = gopts->get_variable_in_option<string>(g4t_item, "unit", "cm");
		st_item.size  = gopts->get_variable_in_option<double>(g4t_item, "size", 24.0);
		st_item.dx    = gopts->get_variable_in_option<double>(g4t_item, "dx", 4.0);
		st_item.dy    = gopts->get_variable_in_option<double>(g4t_item, "dy", 4.0);

		st.push_back(st_item);
	}

	return st;
}

// Define the g4text option schema and help string.
GOptions addSceneTextsOptions() {
	GOptions goptions;
	string   help;

	// g4text
	help = "Adds 2D or 3D text to the current scene. \n \n";
	help += "Example to add two texts: \n \n";
	help += "-g4text=\"[{kind: 2D, text: hello, x: 0.9, y: -0.9, layout: right}, {kind: 3D, text: Shape1, x: 0, y: 6, z: -4}]\"\n";

	vector<GVariable> g4text = {
		{"text", goptions::NODFLT, "string with the text to be displayed"},
		{"color", "black", "color of the text"},
		{"kind", "2D", "text kind: 2D or 3D"},
		{"layout", "", "optional text layout, for example right"},
		{"x", 0, "x position of the text"},
		{"y", 0, "y position of the text"},
		{"z", GNOT_SPECIFIED_SCENE_TEXT_Z, "z position of the text"},
		{"unit", "cm", "unit for 3D text positions"},
		{"size", 24.0, "size of the text"},
		{"dx", 4.0, "3D text x offset"},
		{"dy", 4.0, "3D text y offset"},
	};

	goptions.defineOption("g4text", "Insert texts in the current scene", g4text, help);

	return goptions;
}
} // namespace g4display
