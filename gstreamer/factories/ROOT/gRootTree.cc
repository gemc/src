#include "gRootTree.h"
#include "gstreamerConventions.h"

// clears the maps vectors
bool GRootTree::initTreeForTheEvent() {
	for (auto v : stringVarsMap) { v.second->clear(); }
	for (auto v : intVarsMap) { v.second->clear(); }
	for (auto v : doubleVarsMap) { v.second->clear(); }
	return true;
}

// MARK: Header

// return header tree with initialized leafs
GRootTree::GRootTree(const GEventDataCollectionHeader* gheader, std::shared_ptr<GLogger>& logger) : log(logger) {
	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree header");
	rootTree = new TTree(HEADERTREENAME, HEADERTREENAMESUFFIX);

	registerVariable("g4localEventNumber", gheader->getG4LocalEvn());
	registerVariable("threadID", gheader->getThreadID());
	registerVariable("timeStamp", gheader->getTimeStamp());
}

bool GRootTree::fillTree(const GEventDataCollectionHeader* gheader) {
	intVarsMap["g4localEventNumber"]->push_back(gheader->getG4LocalEvn());
	intVarsMap["threadID"]->push_back(gheader->getThreadID());
	stringVarsMap["timeStamp"]->push_back(gheader->getTimeStamp());

	rootTree->Fill();

	return true;
}


// True Info
GRootTree::GRootTree(const string& detectorName, const GTrueInfoData* gdata, std::shared_ptr<GLogger>& logger) : log(logger) {

	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree True Info");

	string treeName    = TRUEINFONAMEPREFIX + detectorName;
	string description = detectorName + " " + TRUEINFOTREENAMESUFFIX;

	rootTree = new TTree(treeName.c_str(), description.c_str());

	for (auto& [varname, value] : gdata->getDoubleVariablesMap()) { registerVariable(varname, value); }

	for (auto& [varname, value] : gdata->getStringVariablesMap()) { registerVariable(varname, value); }
}

bool GRootTree::fillTree(const vector<GTrueInfoData*>* trueInfoData) {


	for (auto& dataHits : *trueInfoData) {
		// double true info
		for (auto& [varname, value] : dataHits->getDoubleVariablesMap()) { doubleVarsMap[varname]->push_back(value); }

		// string true info
		for (auto& [varname, value] : dataHits->getStringVariablesMap()) { stringVarsMap[varname]->push_back(value); }
	}

	rootTree->Fill();

	return true;
}

// Digitized Data
GRootTree::GRootTree(const string& detectorName, const GDigitizedData* gdata, std::shared_ptr<GLogger>& logger) : log(logger) {

	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree Digitized Data");

	string treeName    = DIGITIZEDNAMEPREFIX + detectorName;
	string description = detectorName + " " + DIGITIZEDTREENAMESUFFIX;

	rootTree = new TTree(treeName.c_str(), description.c_str());

	for (auto& [varname, value] : gdata->getIntObservablesMap(0)) { registerVariable(varname, value); }
	for (auto& [varname, value] : gdata->getDblObservablesMap(0)) { registerVariable(varname, value); }
}


bool GRootTree::fillTree(const vector<GDigitizedData*>* digitizedData) {
	for (auto& dataHits : *digitizedData) {
		for (auto& [varname, value] : dataHits->getIntObservablesMap(0)) { intVarsMap[varname]->push_back(value); }

		for (auto& [varname, value] : dataHits->getDblObservablesMap(0)) { doubleVarsMap[varname]->push_back(value); }
	}
	rootTree->Fill();

	return true;
}

// MARK: Variable Registrations


// instantiate new map entry with its proper type vector and assign it to the root tree branch
// the second argument is needed to select proper overloaded function

void GRootTree::registerVariable(const string& varname, [[maybe_unused]] const int value) {
	if (intVarsMap.find(varname) == intVarsMap.end()) {
		intVarsMap[varname] = new vector<int>;
		rootTree->Branch(varname.c_str(), &intVarsMap[varname]);
	}
	else {
		log->error(ERR_GSTREAMERVARIABLEEXISTS, "variable < ", varname, "< already exist in the int variable map of tree ", rootTree->GetName());
	}
}

void GRootTree::registerVariable(const string& varname, [[maybe_unused]] const double value) {
	if (doubleVarsMap.find(varname) == doubleVarsMap.end()) {
		doubleVarsMap[varname] = new vector<double>;
		rootTree->Branch(varname.c_str(), &doubleVarsMap[varname]);
	}
	else {
		log->error(ERR_GSTREAMERVARIABLEEXISTS, "variable < ", varname, "< already exist in the int variable map of tree ", rootTree->GetName());
	}
}

void GRootTree::registerVariable(const string& varname, [[maybe_unused]] const string& value) {
	if (stringVarsMap.find(varname) == stringVarsMap.end()) {
		stringVarsMap[varname] = new vector<string>;
		rootTree->Branch(varname.c_str(), &stringVarsMap[varname]);
	}
	else {
		log->error(ERR_GSTREAMERVARIABLEEXISTS, "variable < ", varname, "< already exist in the int variable map of tree ", rootTree->GetName());
	}
}
