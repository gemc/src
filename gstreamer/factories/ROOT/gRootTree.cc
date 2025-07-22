#include "gRootTree.h"
#include "gstreamerConventions.h"

using std::string;
using std::vector;


// Return Header Tree with initialized leafs
GRootTree::GRootTree([[maybe_unused]] const std::unique_ptr<GEventHeader>& gheader, std::shared_ptr<GLogger>& logger) : log(logger) {
	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree header");

	// With AUTO FLUSH AND AUTOSAVE
	root_tree = std::make_unique<TTree>(HEADERTREENAME, HEADERTREENAMEDESC);
	root_tree->SetAutoFlush(20 * 1024 * 1024); // write root data buffers to disk automatically once their in-memory size exceeds 20 MB
	root_tree->SetAutoSave(50 * 1024 * 1024);  // save a snapshot of the entire tree (including metadata), useful for recovery after a crash

	registerVariable("g4localEventNumber", gheader->getG4LocalEvn());
	registerVariable("threadID", gheader->getThreadID());
	registerVariable("timeStamp", gheader->getTimeStamp());
}




// Return True Info Tree with initialized leafs
GRootTree::GRootTree(const string& treeName, const GTrueInfoData* gdata, std::shared_ptr<GLogger>& logger) : log(logger) {
	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree True Info");

	// With AUTO FLUSH AND AUTOSAVE
	root_tree = std::make_unique<TTree>(treeName.c_str(), TRUEINFOTREENAMEDESC);
	root_tree->SetAutoFlush(20 * 1024 * 1024); // write root data buffers to disk automatically once their in-memory size exceeds 20 MB
	root_tree->SetAutoSave(50 * 1024 * 1024);  // save a snapshot of the entire tree (including metadata), useful for recovery after a crash

	for (auto& [varname, value] : gdata->getDoubleVariablesMap()) { registerVariable(varname, value); }
	for (auto& [varname, value] : gdata->getStringVariablesMap()) { registerVariable(varname, value); }
}


// Return Digitized Data Tree with initialized leafs
GRootTree::GRootTree(const string& treeName, const GDigitizedData* gdata, std::shared_ptr<GLogger>& logger) : log(logger) {
	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree Digitized Data");

	// With AUTO FLUSH AND AUTOSAVE
	root_tree = std::make_unique<TTree>(treeName.c_str(), DIGITIZEDTREENAMEDESC);
	root_tree->SetAutoFlush(20 * 1024 * 1024); // write root data buffers to disk automatically once their in-memory size exceeds 20 MB
	root_tree->SetAutoSave(50 * 1024 * 1024);  // save a snapshot of the entire tree (including metadata), useful for recovery after a crash

	for (auto& [varname, value] : gdata->getIntObservablesMap(0)) { registerVariable(varname, value); }
	for (auto& [varname, value] : gdata->getDblObservablesMap(0)) { registerVariable(varname, value); }
}

// fill the header tree
bool GRootTree::fillTree(const std::unique_ptr<GEventHeader>& gheader) {
	log->info(0, "Filling header tree for local event n. ", gheader->getG4LocalEvn(), " threadID ", gheader->getThreadID());

	// clearing previous header info
	intVarsMap["g4localEventNumber"].clear();
	intVarsMap["threadID"].clear();
	stringVarsMap["timeStamp"].clear();

	intVarsMap["g4localEventNumber"].emplace_back(gheader->getG4LocalEvn());
	intVarsMap["threadID"].emplace_back(gheader->getThreadID());
	stringVarsMap["timeStamp"].emplace_back(gheader->getTimeStamp());

	root_tree->Fill();

	return true;
}

// fill the True Info Tree
bool GRootTree::fillTree(const vector<const GTrueInfoData*>& trueInfoData) {

	// clearing previous true info
	for (auto& [varname, values] : doubleVarsMap) { values.clear(); }
	for (auto& [varname, values] : stringVarsMap) { values.clear(); }


	for (auto& dataHits : trueInfoData) {

		// double true info
		for (auto& [varname, value] : dataHits->getDoubleVariablesMap()) { doubleVarsMap[varname].push_back(value); }

		// string true info
		for (auto& [varname, value] : dataHits->getStringVariablesMap()) { stringVarsMap[varname].push_back(value); }
	}

	root_tree->Fill();

	return true;
}

// fill the Digitized Data Tree
bool GRootTree::fillTree(const vector<const GDigitizedData*>& digitizedData) {

	// clearing previous digitized data
	for (auto& [varname, values] : intVarsMap) { values.clear(); }
	for (auto& [varname, values] : doubleVarsMap) { values.clear(); }

	for (auto& dataHits : digitizedData) {
		// int digitized data
		for (auto& [varname, value] : dataHits->getIntObservablesMap(0)) { intVarsMap[varname].push_back(value); }

		// double digitized data
		for (auto& [varname, value] : dataHits->getDblObservablesMap(0)) { doubleVarsMap[varname].push_back(value); }
	}
	root_tree->Fill();

	return true;
}


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
