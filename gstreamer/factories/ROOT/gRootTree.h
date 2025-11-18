#pragma once

// c++
#include <map>

// ROOT
#include "TTree.h"
#include "ROOT/TThreadedObject.hxx"

// gemc
#include "event/gEventDataCollection.h"

#include "gstreamerConventions.h"

#define HEADERTREENAME "event_header"
#define TRUEINFONAMEPREFIX   "true_info_"
#define DIGITIZEDNAMEPREFIX  "digitized_"

#define ERR_GSTREAMERROOTTREENOTFOUND   850

#define HEADERTREENAMEDESC    "Event Header"
#define TRUEINFOTREENAMEDESC  "True Info Data"
#define DIGITIZEDTREENAMEDESC "Digitized Data"

// connection between gdata and root
class GRootTree {
public:
	~GRootTree() { if (log) log->debug(NORMAL, "~GRootTree"); }

	// types of TTree
	GRootTree([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header, std::shared_ptr<GLogger>& log);
	GRootTree(const std::string& detectorName, const GTrueInfoData* gdata, std::shared_ptr<GLogger>& log);
	GRootTree(const std::string& detectorName, const GDigitizedData* gdata, std::shared_ptr<GLogger>& log);

	// filling trees
	bool fillTree(const std::unique_ptr<GEventHeader>& gevent_header);
	bool fillTree(const std::vector<const GTrueInfoData*>& trueInfoData);
	bool fillTree(const std::vector<const GDigitizedData*>& digitizedData);

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
