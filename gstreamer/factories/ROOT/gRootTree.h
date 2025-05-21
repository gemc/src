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
	GRootTree(const std::string& detectorName, const GTrueInfoData* gdata, std::shared_ptr<GLogger>& log);
	GRootTree(const std::string& detectorName, const GDigitizedData* gdata, std::shared_ptr<GLogger>& log);

	// filling trees
	bool fillTree(const GEventDataCollectionHeader* gheader);
	bool fillTree(const std::vector<GTrueInfoData*>* trueInfoData);
	bool fillTree(const std::vector<GDigitizedData*>* digitizedData);

	// clear variables map below
	bool initTreeForTheEvent();

private:
	TTree* rootTree = nullptr;

	// variable maps
	// index is hit number
	std::map<std::string, std::vector<int>*>    intVarsMap;
	std::map<std::string, std::vector<double>*> doubleVarsMap;
	std::map<std::string, std::vector<std::string>*> stringVarsMap;


	// the second argument is needed to select the VarsMap type and its content
	void registerVariable(const std::string& varname, int value);
	void registerVariable(const std::string& varname, double value);
	void registerVariable(const std::string& varname, const std::string& value);

	std::shared_ptr<GLogger>& log;
};
