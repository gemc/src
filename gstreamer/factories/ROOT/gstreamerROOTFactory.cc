// gstreamer
#include "gRootTree.h"
#include "gstreamerROOTFactory.h"

using std::string;

// Return the header tree from the map. If it's not there, initialize it.
GRootTree* GstreamerRootFactory::getOrInstantiateHeaderTree(const GEventDataCollectionHeader* gheader,
                                                             std::shared_ptr<GLogger>&   log) {
	// the tree is not found, initialize it
	if (gRootTrees->find(HEADERTREENAME) == gRootTrees->end()) {
		log->info(2, "GstreamerRootFactory", "Creating ROOT header tree");
		(*gRootTrees)[HEADERTREENAME] = new GRootTree(gheader, log);
	}
	return (*gRootTrees)[HEADERTREENAME];
}

// gdata passed here is guaranteed not a nullptr
GRootTree* GstreamerRootFactory::getOrInstantiateTrueInfoDataTree(const string&                    detectorName,
                                                                  const GTrueInfoData*            gdata,
                                                                   std::shared_ptr<GLogger>& log) {
	string treeName = detectorName + TRUEINFONAMEPREFIX;

	// the tree is not found, initialize it
	if (gRootTrees->find(treeName) == gRootTrees->end()) {
		log->info(2, "GstreamerRootFactory", "Creating ROOT true info tree for detector: " + detectorName);
		(*gRootTrees)[treeName] = new GRootTree(detectorName, gdata, log);
	}

	return (*gRootTrees)[treeName];
}

// gdata passed here is guaranteed not a nullptr
GRootTree* GstreamerRootFactory::getOrInstantiateDigitizedDataTree(const string&                    detectorName,
                                                                   const GDigitizedData*           gdata,
                                                                    std::shared_ptr<GLogger>& log) {
	string treeName = detectorName + DIGITIZEDNAMEPREFIX;

	// the tree is not found, initialize it
	if (gRootTrees->find(treeName) == gRootTrees->end()) {
		log->info(2, "GstreamerRootFactory", "Creating ROOT digitized data tree for detector: " + detectorName);
		(*gRootTrees)[treeName] = new GRootTree(detectorName, gdata, log);
	}

	return (*gRootTrees)[treeName];
}


// tells the DLL how to create a GStreamerFactory
extern "C" GStreamer* GStreamerFactory(void) { return static_cast<GStreamer*>(new GstreamerRootFactory); }
