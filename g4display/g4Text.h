#ifndef  G4TEXT_H_OPTIONS
#define  G4TEXT_H_OPTIONS 1

// glibrary
#include "goptions.h"


namespace g4display {

	// JSceneText: Text does not move with the view but belons to the scene
	struct JSceneText {
		string text;
		string color;
		float x;
		float y;
		int size;
	};

	void from_json(const json& j, JSceneText& jstext);

	// method to return a vector of JSystem from a structured option
	vector<JSceneText> getSceneTexts(GOptions *gopts);

	// returns the array of options definitions
	vector<GOption> addSceneTextsOptions();


	// JSceneText: Text does not move with the view but belons to the scene
	struct JViewText {
		string text;
		string color;
		float x;
		float y;
		float z;
		int size;
	};

	void from_json(const json& j, JViewText& jvtext);

	// method to return a vector of JSystem from a structured option
	vector<JViewText> getViewTexts(GOptions *gopts);

	// returns the array of options definitions
	vector<GOption> addSceneTextsOptions();
	vector<GOption> addViewTextsOptions();




}

#endif

