// gstreamer
#include "gRootTree.h"
#include "gstreamerROOTFactory.h"

// return header tree from map. If not there, initialize it.
GRootTree* GstreamerRootFactory::getOrInstantiateHeaderTree(const GEventDataCollectionHeader *gheader) {

	// tree not found, initializing it
	if(gRootTrees->find(HEADERTREENAME) == gRootTrees->end()) {
		(*gRootTrees)[HEADERTREENAME] = new GRootTree(gheader);
	}

	// return initialized tree
	return (*gRootTrees)[HEADERTREENAME];

}

// gdata passed here is guaranteed not a nullptr
GRootTree* GstreamerRootFactory::getOrInstantiateTrueInfoDataTree(const string detectorName, const GTrueInfoData*  gdata){
	string treeName = detectorName + TRUEINFONAMEPREFIX;

	// tree not found, initializing it
	if(gRootTrees->find(treeName) == gRootTrees->end() ) {
		(*gRootTrees)[treeName] = new GRootTree(detectorName, gdata);
	}

	// return initialized tree
	return (*gRootTrees)[treeName];

}

// gdata passed here is guaranteed not a nullptr
GRootTree* GstreamerRootFactory::getOrInstantiateDigitizedDataTree(const string detectorName, const GDigitizedData* gdata) {
	string treeName = detectorName + DIGITIZEDNAMEPREFIX;

	// tree not found, initializing it
	if(gRootTrees->find(treeName) == gRootTrees->end() ) {
		(*gRootTrees)[treeName] = new GRootTree(detectorName, gdata);
	}

	// return initialized tree
	return (*gRootTrees)[treeName];

}


// tells the DLL how to create a GStreamerFactory
extern "C" GStreamer* GStreamerFactory(void) {
	return static_cast<GStreamer*>(new GstreamerRootFactory);
}
