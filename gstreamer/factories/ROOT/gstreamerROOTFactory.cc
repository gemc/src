// gstreamer
#include "gRootTree.h"
#include "gstreamerROOTFactory.h"

// keeps the include independent of TROOT
#include <ROOT/RConfig.hxx>

// enable root thread safety in a static call that
// runs before control returns from dlopen() as soon as libGstreamerRootFactory.so is loaded.
namespace {
struct EnableRootTS {
	EnableRootTS() {
		ROOT::EnableThreadSafety();
		std::cout << "GstreamerRootFactory: ROOT thread safety enabled" << std::endl;
	}
};

static EnableRootTS _enableROOTLocks; // global object, static storage duration.
} // unnamed namespace

// Return the header tree pointer from the map. If it's not there, initialize the smart pointer.
const std::unique_ptr<GRootTree>& GstreamerRootFactory::getOrInstantiateHeaderTree([[maybe_unused]] const std::unique_ptr<GEventHeader>& event_header) {
	rootfile->cd();

	if (!log) {
		std::cerr << "FATAL: log is null in GstreamerRootFactory::getOrInstantiateHeaderTree" << std::endl;
		std::terminate();
	}

	if (!event_header) { log->error(ERR_PUBLISH_ERROR, "event header is null in GstreamerRootFactory::getOrInstantiateHeaderTree"); }

	// If the key does not exist, this inserts a new entry with a default value
	// and returns a reference to it.
	auto& treePtr = gRootTrees[HEADERTREENAME];
	if (!treePtr) {
		log->info(2, "GstreamerRootFactory", "Creating ROOT", HEADERTREENAME, " tree");
		treePtr = std::make_unique<GRootTree>(event_header, log); // add the new tree to the map
	}

	return treePtr;
}

// gdata passed here is guaranteed not a nullptr
const std::unique_ptr<GRootTree>& GstreamerRootFactory::getOrInstantiateTrueInfoDataTree(const std::string&   detectorName,
                                                                                         const GTrueInfoData* gdata) {
	std::string treeName = TRUEINFONAMEPREFIX + detectorName;

	auto& treePtr = gRootTrees[treeName];
	if (!treePtr) {
		log->info(2, "GstreamerRootFactory", "Creating GTrueInfoData ROOT tree for ", detectorName);
		treePtr = std::make_unique<GRootTree>(treeName, gdata, log); // add the new tree to the map
	}

	return treePtr;
}

// gdata passed here is guaranteed not a nullptr
const std::unique_ptr<GRootTree>& GstreamerRootFactory::getOrInstantiateDigitizedDataTree(const std::string&    detectorName,
                                                                                          const GDigitizedData* gdata) {
	std::string treeName = DIGITIZEDNAMEPREFIX + detectorName;

	auto& treePtr = gRootTrees[treeName];
	if (!treePtr) {
		log->info(2, "GstreamerRootFactory", "Creating GDigitizedData ROOT tree for ", detectorName);
		treePtr = std::make_unique<GRootTree>(treeName, gdata, log); // add the new tree to the map
	}

	return treePtr;
}



// tells the DLL how to create a GStreamerFactory in each plugin .so/.dylib
extern "C" GStreamer* GStreamerFactory(const std::shared_ptr<GOptions>& g) {
	return  static_cast<GStreamer*> (new GstreamerRootFactory(g));
}
