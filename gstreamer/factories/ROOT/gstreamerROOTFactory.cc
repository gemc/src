// gstreamer
#include "gRootTree.h"
#include "gstreamerROOTFactory.h"

// keeps the include independent of TROOT
#include <ROOT/RConfig.hxx>

// Implementation summary:
// Enable ROOT thread safety at plugin load time and provide lazy tree-creation helpers.

namespace {
	struct EnableRootTS
	{
		EnableRootTS() {
			ROOT::EnableThreadSafety();
			std::cout << "GstreamerRootFactory: ROOT thread safety enabled" << std::endl;
		}
	};

	static EnableRootTS _enableROOTLocks;
}

// Implementation summary:
// Return the event-header tree, creating it on first use.
const std::unique_ptr<GRootTree>& GstreamerRootFactory::getOrInstantiateHeaderTree(
	[[maybe_unused]] const std::unique_ptr<GEventHeader>& event_header) {
	rootfile->cd();

	if (!log) {
		std::cerr << "FATAL: log is null in GstreamerRootFactory::getOrInstantiateHeaderTree" << std::endl;
		std::terminate();
	}

	if (!event_header) {
		log->error(ERR_PUBLISH_ERROR, "event header is null in GstreamerRootFactory::getOrInstantiateHeaderTree");
	}

	// operator[] either returns the existing entry or creates an empty one.
	auto& treePtr = gRootTrees[EVENTHEADERTREENAME];
	if (!treePtr) {
		log->info(2, "GstreamerRootFactory", "Creating ROOT", EVENTHEADERTREENAME, " tree");
		treePtr = std::make_unique<GRootTree>(event_header, log);
	}

	return treePtr;
}

// Implementation summary:
// Return the run-header tree, creating it on first use.
const std::unique_ptr<GRootTree>& GstreamerRootFactory::getOrInstantiateHeaderTree(
	[[maybe_unused]] const std::unique_ptr<GRunHeader>& run_header) {
	rootfile->cd();

	if (!log) {
		std::cerr << "FATAL: log is null in GstreamerRootFactory::getOrInstantiateHeaderTree" << std::endl;
		std::terminate();
	}

	if (!run_header) {
		log->error(ERR_PUBLISH_ERROR, "run header is null in GstreamerRootFactory::getOrInstantiateHeaderTree");
	}

	auto& treePtr = gRootTrees[RUNHEADERTREENAME];
	if (!treePtr) {
		log->info(2, "GstreamerRootFactory", "Creating ROOT", RUNHEADERTREENAME, " tree");
		treePtr = std::make_unique<GRootTree>(run_header, log);
	}

	return treePtr;
}

// Implementation summary:
// Return the true-information detector tree, creating it lazily from the first sample hit.
const std::unique_ptr<GRootTree>& GstreamerRootFactory::getOrInstantiateTrueInfoDataTree(
	const std::string&   detectorName,
	const GTrueInfoData* gdata) {
	std::string treeName = TRUEINFONAMEPREFIX + detectorName;

	auto& treePtr = gRootTrees[treeName];
	if (!treePtr) {
		log->info(2, "GstreamerRootFactory", "Creating GTrueInfoData ROOT tree for ", detectorName);
		treePtr = std::make_unique<GRootTree>(treeName, gdata, log);
	}

	return treePtr;
}

// Implementation summary:
// Return the digitized detector tree, creating it lazily from the first sample hit.
const std::unique_ptr<GRootTree>& GstreamerRootFactory::getOrInstantiateDigitizedDataTree(
	const std::string&    detectorName,
	const GDigitizedData* gdata) {
	std::string treeName = DIGITIZEDNAMEPREFIX + detectorName;

	auto& treePtr = gRootTrees[treeName];
	if (!treePtr) {
		log->info(2, "GstreamerRootFactory", "Creating GDigitizedData ROOT tree for ", detectorName);
		treePtr = std::make_unique<GRootTree>(treeName, gdata, log);
	}

	return treePtr;
}


// Implementation summary:
// Export the factory symbol required by the plugin loader.
extern "C" GStreamer* GStreamerFactory(const std::shared_ptr<GOptions>& g) {
	return static_cast<GStreamer*>(new GstreamerRootFactory(g));
}