#pragma once

// c++
#include <map>

// ROOT
#include "TTree.h"

// gemc
#include "event/gEventDataCollection.h"

#define TRUEINFONAMEPREFIX   "trueInfo_"
#define DIGITIZEDNAMEPREFIX  "digitized_"

#define ERR_GSTREAMERROOTTREENOTFOUND   850

#define HEADERTREENAME "header"

#define HEADERTREENAMESUFFIX    "GEMC Root Event Header"
#define TRUEINFOTREENAMESUFFIX  "True Info Data Root Tree"
#define DIGITIZEDTREENAMESUFFIX "Digitized Data Root Tree"

// connection between gdata and root
class GRootTree {
public:
	// types of TTree
	GRootTree(const GEventDataCollectionHeader* gheader, std::shared_ptr<GLogger>& log);
	GRootTree(const string& detectorName, const GTrueInfoData* gdata, std::shared_ptr<GLogger>& log);
	GRootTree(const string& detectorName, const GDigitizedData* gdata, std::shared_ptr<GLogger>& log);

	// filling trees
	bool fillTree(const GEventDataCollectionHeader* gheader);
	bool fillTree(const vector<GTrueInfoData*>* trueInfoData);
	bool fillTree(const vector<GDigitizedData*>* digitizedData);

	// clear variables map below
	bool initTreeForTheEvent();

private:
	TTree* rootTree = nullptr;

	// variable maps
	// index is hit number
	map<string, vector<int>*>    intVarsMap;
	map<string, vector<double>*> doubleVarsMap;
	map<string, vector<string>*> stringVarsMap;


	// the second argument is needed to select the VarsMap type and its content
	void registerVariable(const string& varname, int value);
	void registerVariable(const string& varname, double value);
	void registerVariable(const string& varname, const string& value);

	std::shared_ptr<GLogger>& log;
};
