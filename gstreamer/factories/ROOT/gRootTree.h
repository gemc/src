#pragma once

// c++
#include <map>

// ROOT
#include "TTree.h"
#include "ROOT/TThreadedObject.hxx"

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
	GRootTree(const GEventHeader* gheader, std::shared_ptr<GLogger>& log);
	GRootTree(const std::string& detectorName, const GTrueInfoData* gdata, std::shared_ptr<GLogger>& log);
	GRootTree(const std::string& detectorName, const GDigitizedData* gdata, std::shared_ptr<GLogger>& log);

	// filling trees
	bool fillTree(const GEventHeader* gheader);
	bool fillTree(const std::vector<GTrueInfoData*>* trueInfoData);
	bool fillTree(const std::vector<GDigitizedData*>* digitizedData);

	// write ROOT tree to file
	void writeToFile() {
		if (root_tree->GetDirectory() == nullptr) {
			log->warning(1, "ROOT tree ", root_tree->GetName(), " has no directory. Skipping Write().");
			return;
		}

		if (root_tree) root_tree->Write();
	}

private:
	std::unique_ptr<TTree> root_tree;


	// variable maps
	// index is hit number
	std::map<std::string, std::vector<int>>         intVarsMap;
	std::map<std::string, std::vector<double>>      doubleVarsMap;
	std::map<std::string, std::vector<std::string>> stringVarsMap;


	// the second argument is needed to select the VarsMap type and its content
	void registerVariable(const std::string& varname, int value);
	void registerVariable(const std::string& varname, double value);
	void registerVariable(const std::string& varname, const std::string& value);

	std::shared_ptr<GLogger>& log;
};
