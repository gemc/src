// gstreamer
#include "gRootTree.h"
#include "gstreamerROOTFactory.h"

using std::string;

// Return the header tree from the map. If it's not there, initialize it.
GRootTree* GstreamerRootFactory::getOrInstantiateHeaderTree(const GEventDataCollectionHeader* gheader) {
	// the tree is not found, initialize it
	if (gRootTrees.find(HEADERTREENAME) == gRootTrees.end()) {
		log->info(2, "GstreamerRootFactory", "Creating ROOT header tree");
		gRootTrees[HEADERTREENAME] = std::make_unique<GRootTree>(gheader, log);
	}
	return gRootTrees[HEADERTREENAME].get();
}

// gdata passed here is guaranteed not a nullptr
GRootTree* GstreamerRootFactory::getOrInstantiateTrueInfoDataTree(const string&        detectorName,
                                                                  const GTrueInfoData* gdata) {
	string treeName = detectorName + TRUEINFONAMEPREFIX;

	// the tree is not found, initialize it
	if (gRootTrees.find(treeName) == gRootTrees.end()) {
		log->info(2, "GstreamerRootFactory", "Creating ROOT true info tree for detector: " + detectorName);
		gRootTrees[treeName] = std::make_unique<GRootTree>(detectorName, gdata, log);
	}

	return gRootTrees[treeName].get();  // transfer ownership to caller
}

// gdata passed here is guaranteed not a nullptr
GRootTree* GstreamerRootFactory::getOrInstantiateDigitizedDataTree(const string&         detectorName,
                                                                   const GDigitizedData* gdata) {
	string treeName = detectorName + DIGITIZEDNAMEPREFIX;

	// the tree is not found, initialize it
	if (gRootTrees.find(treeName) == gRootTrees.end()) {
		log->info(2, "GstreamerRootFactory", "Creating ROOT digitized data tree for detector: " + detectorName);
		gRootTrees[treeName] = std::make_unique<GRootTree>(detectorName, gdata, log);
	}

	return gRootTrees[treeName].get();  // transfer ownership to caller
}


// tells the DLL how to create a GStreamerFactory
extern "C" GStreamer* GStreamerFactory(void) { return static_cast<GStreamer*>(new GstreamerRootFactory); }
