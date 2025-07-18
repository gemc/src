#include "gRootTree.h"
#include "gstreamerConventions.h"

using std::string;
using std::vector;


// return header tree with initialized leafs
GRootTree::GRootTree([[maybe_unused]] const std::unique_ptr<GEventHeader>& gheader, std::shared_ptr<GLogger>& logger) : log(logger) {

	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree header");

	root_tree = std::make_unique<TTree>(HEADERTREENAME, HEADERTREENAMESUFFIX);

	registerVariable("g4localEventNumber", gheader->getG4LocalEvn());
	registerVariable("threadID", gheader->getThreadID());
	registerVariable("timeStamp", gheader->getTimeStamp());
}

bool GRootTree::fillTree(const std::unique_ptr<GEventHeader>& gheader) {

	log->info(0, "Filling header tree for event ", gheader->getG4LocalEvn(), " threadID ", gheader->getThreadID());

	intVarsMap["g4localEventNumber"].emplace_back(gheader->getG4LocalEvn());
	intVarsMap["threadID"].emplace_back(gheader->getThreadID());
	stringVarsMap["timeStamp"].emplace_back(gheader->getTimeStamp());

	root_tree->Fill();

	return true;
}


// True Info
// GRootTree::GRootTree(const string& detectorName, const GTrueInfoData* gdata, std::shared_ptr<GLogger>& logger) : log(logger) {
// 	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree True Info");
//
// 	string treeName    = TRUEINFONAMEPREFIX + detectorName;
// 	string description = detectorName + " " + TRUEINFOTREENAMESUFFIX;
//
// 	root_tree = std::make_unique<TTree>(treeName.c_str(), description.c_str());
//
// 	for (auto& [varname, value] : gdata->getDoubleVariablesMap()) { registerVariable(varname, value); }
//
// 	for (auto& [varname, value] : gdata->getStringVariablesMap()) { registerVariable(varname, value); }
// }
//
//
//
// // Digitized Data
// GRootTree::GRootTree(const string& detectorName, const GDigitizedData* gdata, std::shared_ptr<GLogger>& logger) : log(logger) {
// 	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree Digitized Data");
//
// 	string treeName    = DIGITIZEDNAMEPREFIX + detectorName;
// 	string description = detectorName + " " + DIGITIZEDTREENAMESUFFIX;
//
// 	root_tree = std::make_unique<TTree>(treeName.c_str(), description.c_str());
//
// 	for (auto& [varname, value] : gdata->getIntObservablesMap(0)) { registerVariable(varname, value); }
// 	for (auto& [varname, value] : gdata->getDblObservablesMap(0)) { registerVariable(varname, value); }
// }


// bool GRootTree::fillTree(const vector<GTrueInfoData*>* trueInfoData) {
// 	for (auto& dataHits : *trueInfoData) {
// 		// double true info
// 		for (auto& [varname, value] : dataHits->getDoubleVariablesMap()) { doubleVarsMap[varname].push_back(value); }
//
// 		// string true info
// 		for (auto& [varname, value] : dataHits->getStringVariablesMap()) { stringVarsMap[varname].push_back(value); }
// 	}
//
// 	root_tree->Fill();
//
// 	return true;
// }

// bool GRootTree::fillTree(const vector<GDigitizedData*>* digitizedData) {
// 	for (auto& dataHits : *digitizedData) {
// 		for (auto& [varname, value] : dataHits->getIntObservablesMap(0)) { intVarsMap[varname].push_back(value); }
//
// 		for (auto& [varname, value] : dataHits->getDblObservablesMap(0)) { doubleVarsMap[varname].push_back(value); }
// 	}
// 	root_tree->Fill();
//
// 	return true;
// }

// MARK: Variable Registrations


// instantiate new map entry with its proper type vector and assign it to the root tree branch
// the second argument is needed to select proper overloaded function

void GRootTree::registerVariable(const string& varname, [[maybe_unused]] const int value) {
	if (intVarsMap.find(varname) == intVarsMap.end()) { root_tree->Branch(varname.c_str(), &intVarsMap[varname]); }
	else { log->error(ERR_GSTREAMERVARIABLEEXISTS, "variable < ", varname, "< already exist in the int variable map of tree ", root_tree->GetName()); }
}

void GRootTree::registerVariable(const string& varname, [[maybe_unused]] const double value) {
	if (doubleVarsMap.find(varname) == doubleVarsMap.end()) { root_tree->Branch(varname.c_str(), &doubleVarsMap[varname]); }
	else { log->error(ERR_GSTREAMERVARIABLEEXISTS, "variable < ", varname, "< already exist in the int variable map of tree ", root_tree->GetName()); }
}

void GRootTree::registerVariable(const string& varname, [[maybe_unused]] const string& value) {
	if (stringVarsMap.find(varname) == stringVarsMap.end()) { root_tree->Branch(varname.c_str(), &stringVarsMap[varname]); }
	else { log->error(ERR_GSTREAMERVARIABLEEXISTS, "variable < ", varname, "< already exist in the int variable map of tree ", root_tree->GetName()); }
}
