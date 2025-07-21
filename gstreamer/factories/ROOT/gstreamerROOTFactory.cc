// gstreamer
#include "gRootTree.h"
#include "gstreamerROOTFactory.h"

// Return the header tree pointer from the map. If it's not there, initialize the smart pointer.
GRootTree* GstreamerRootFactory::getOrInstantiateHeaderTree([[maybe_unused]] const std::unique_ptr<GEventHeader>& gheader) {

	rootfile->cd();

	if (!log) {
		std::cerr << "FATAL: log is null in GstreamerRootFactory::getOrInstantiateHeaderTree" << std::endl;
		std::terminate();
	}

	if (!gheader) {
		log->error(ERR_PUBLISH_ERROR, "event header is null in GstreamerRootFactory::getOrInstantiateHeaderTree");
	}

	// If the key does not exist, this inserts a new entry with default value
	// and returns a reference to it.
	auto& treePtr = gRootTrees[HEADERTREENAME];
	if (!treePtr) {
		log->info(2, "GstreamerRootFactory", "Creating ROOT", HEADERTREENAME, " tree");
		treePtr = std::make_unique<GRootTree>(gheader, log); // add new tree to the map
	}

	return treePtr.get();

}

// gdata passed here is guaranteed not a nullptr
// std::unique_ptr<GRootTree> GstreamerRootFactory::getOrInstantiateTrueInfoDataTree(const std::string&   detectorName,
//                                                                                   const GTrueInfoData* gdata) {
// 	std::string treeName = detectorName + TRUEINFONAMEPREFIX;
//
// 	// the tree is not found, initialize it
// 	if (gRootTrees.find(treeName) == gRootTrees.end()) {
// 		log->info(2, "GstreamerRootFactory", "Creating ROOT true info tree for detector: " + detectorName);
// 		gRootTrees[treeName] = std::make_unique<GRootTree>(detectorName, gdata, log);
// 	}
//
// 	return gRootTrees[treeName].get(); // transfer ownership to caller
// }
//
// // gdata passed here is guaranteed not a nullptr
// std::unique_ptr<GRootTree> GstreamerRootFactory::getOrInstantiateDigitizedDataTree(const std::string&    detectorName,
//                                                                                    const GDigitizedData* gdata) {
// 	std::string treeName = detectorName + DIGITIZEDNAMEPREFIX;
//
// 	// the tree is not found, initialize it
// 	if (gRootTrees.find(treeName) == gRootTrees.end()) {
// 		log->info(2, "GstreamerRootFactory", "Creating ROOT digitized data tree for detector: " + detectorName);
// 		gRootTrees[treeName] = std::make_unique<GRootTree>(detectorName, gdata, log);
// 	}
//
// 	return gRootTrees[treeName].get(); // transfer ownership to caller
// }


// tells the DLL how to create a GStreamerFactory
extern "C" GStreamer* GStreamerFactory(void) { return static_cast<GStreamer*>(new GstreamerRootFactory); }
