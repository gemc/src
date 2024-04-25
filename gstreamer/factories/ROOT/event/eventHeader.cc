// gstreamer
#include "../gstreamerROOTFactory.h"


bool GstreamerRootFactory::publishEventHeader(const GEventDataCollectionHeader *gheader) {
    if (rootfile == nullptr) return false;

    // get or instantiate root tree from the map
    GRootTree *headerTree = getOrInstantiateHeaderTree(gheader);

    // fill variables and tree
    return headerTree->fillTree(gheader);
}
