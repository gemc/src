#include "gRootTree.h"
#include "gstreamerConventions.h"

using std::vector;

// Implementation summary:
// Build ROOT TTrees lazily from sample headers or hits, then clear and refill
// vector branches on each fill call.

GRootTree::GRootTree([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header,
					 std::shared_ptr<GLogger>&                             logger) : log(logger) {
	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree header");

	root_tree = std::make_unique<TTree>(EVENTHEADERTREENAME, EVENTHEADERTREENAMEDESC);

	// AutoFlush controls when buffered basket data are pushed to disk.
	root_tree->SetAutoFlush(20 * 1024 * 1024);

	// AutoSave periodically writes tree metadata snapshots for recoverability.
	root_tree->SetAutoSave(50 * 1024 * 1024);

	registerVariable("g4localEventNumber", gevent_header->getG4LocalEvn());
	registerVariable("threadID", gevent_header->getThreadID());
	registerVariable("timeStamp", gevent_header->getTimeStamp());
}


GRootTree::GRootTree([[maybe_unused]] const std::unique_ptr<GRunHeader>& grun_header,
					 std::shared_ptr<GLogger>&                           logger)  : log(logger) {

	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree header");

	root_tree = std::make_unique<TTree>(RUNHEADERTREENAME, RUNHEADERTREENAMEDESC);
	root_tree->SetAutoFlush(20 * 1024 * 1024);
	root_tree->SetAutoSave(50 * 1024 * 1024);

	registerVariable("runID", grun_header->getRunID());
}


// Implementation summary:
// Create a true-information detector tree whose branch schema is inferred
// from the first observed hit.
GRootTree::GRootTree(const std::string&        detectorName,
					 const GTrueInfoData*      gdata,
					 std::shared_ptr<GLogger>& logger) : log(logger) {
	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree True Info");

	root_tree = std::make_unique<TTree>(detectorName.c_str(), TRUEINFOTREENAMEDESC);
	root_tree->SetAutoFlush(20 * 1024 * 1024);
	root_tree->SetAutoSave(50 * 1024 * 1024);

	for (auto& [varname, value] : gdata->getDoubleVariablesMap()) { registerVariable(varname, value); }
	for (auto& [varname, value] : gdata->getStringVariablesMap()) { registerVariable(varname, value); }
}


// Implementation summary:
// Create a digitized detector tree whose branch schema is inferred
// from the first observed hit.
GRootTree::GRootTree(const std::string&        detectorName,
					 const GDigitizedData*     gdata,
					 std::shared_ptr<GLogger>& logger) : log(logger) {
	log->debug(CONSTRUCTOR, "GRootTree", "ROOT tree Digitized Data");

	root_tree = std::make_unique<TTree>(detectorName.c_str(), DIGITIZEDTREENAMEDESC);
	root_tree->SetAutoFlush(20 * 1024 * 1024);
	root_tree->SetAutoSave(50 * 1024 * 1024);

	for (auto& [varname, value] : gdata->getIntObservablesMap(0)) { registerVariable(varname, value); }
	for (auto& [varname, value] : gdata->getDblObservablesMap(0)) { registerVariable(varname, value); }
}

// fill the header tree
bool GRootTree::fillTree(const std::unique_ptr<GEventHeader>& gevent_header) {
	log->info(2, "Filling header tree for local event n. ", gevent_header->getG4LocalEvn(), " threadID ",
			  gevent_header->getThreadID());

	// Clear the vectors backing the branches before writing the next entry.
	intVarsMap["g4localEventNumber"].clear();
	intVarsMap["threadID"].clear();
	stringVarsMap["timeStamp"].clear();

	intVarsMap["g4localEventNumber"].emplace_back(gevent_header->getG4LocalEvn());
	intVarsMap["threadID"].emplace_back(gevent_header->getThreadID());
	stringVarsMap["timeStamp"].emplace_back(gevent_header->getTimeStamp());

	root_tree->Fill();

	return true;
}


bool GRootTree::fillTree(const std::unique_ptr<GRunHeader>& grun_header) {
	log->info(2, "Filling header tree for run n. ", grun_header->getRunID());

	// Clear and refill the vectors backing the run-header branches.
	intVarsMap["runID"].clear();
	intVarsMap["runID"].emplace_back(grun_header->getRunID());

	root_tree->Fill();

	return true;
}

// fill the True Info Tree
bool GRootTree::fillTree(const vector<const GTrueInfoData*>& trueInfoData) {
	// Reset all branch vectors before repopulating them for this detector collection.
	for (auto& [varname, values] : doubleVarsMap) { values.clear(); }
	for (auto& [varname, values] : stringVarsMap) { values.clear(); }

	for (auto& dataHits : trueInfoData) {
		for (auto& [varname, value] : dataHits->getDoubleVariablesMap()) { doubleVarsMap[varname].push_back(value); }
		for (auto& [varname, value] : dataHits->getStringVariablesMap()) { stringVarsMap[varname].push_back(value); }
	}

	root_tree->Fill();

	return true;
}

// fill the Digitized Data Tree
bool GRootTree::fillTree(const vector<const GDigitizedData*>& digitizedData) {
	// Reset all branch vectors before repopulating them for this detector collection.
	for (auto& [varname, values] : intVarsMap) { values.clear(); }
	for (auto& [varname, values] : doubleVarsMap) { values.clear(); }

	for (auto& dataHits : digitizedData) {
		for (auto& [varname, value] : dataHits->getIntObservablesMap(0)) { intVarsMap[varname].push_back(value); }
		for (auto& [varname, value] : dataHits->getDblObservablesMap(0)) { doubleVarsMap[varname].push_back(value); }
	}
	root_tree->Fill();

	return true;
}


// Implementation summary:
// Register one branch and bind it to the appropriate backing vector map.
// The second parameter is used only to select the correct overload.

void GRootTree::registerVariable(const std::string& varname, [[maybe_unused]] int value) {
	if (intVarsMap.find(varname) == intVarsMap.end()) { root_tree->Branch(varname.c_str(), &intVarsMap[varname]); }
	else {
		log->error(ERR_GSTREAMERVARIABLEEXISTS, "variable < ", varname,
				   "< already exist in the int variable map of tree ", root_tree->GetName());
	}
}

void GRootTree::registerVariable(const std::string& varname, [[maybe_unused]] double value) {
	if (doubleVarsMap.find(varname) == doubleVarsMap.end()) {
		root_tree->Branch(varname.c_str(), &doubleVarsMap[varname]);
	}
	else {
		log->error(ERR_GSTREAMERVARIABLEEXISTS, "variable < ", varname,
				   "< already exist in the double variable map of tree ", root_tree->GetName());
	}
}

void GRootTree::registerVariable(const std::string& varname, [[maybe_unused]] const std::string& value) {
	if (stringVarsMap.find(varname) == stringVarsMap.end()) {
		root_tree->Branch(varname.c_str(), &stringVarsMap[varname]);
	}
	else {
		log->error(ERR_GSTREAMERVARIABLEEXISTS, "variable < ", varname,
				   "< already exist in the string variable map of tree ", root_tree->GetName());
	}
}