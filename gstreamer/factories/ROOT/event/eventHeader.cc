// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"


bool GstreamerRootFactory::publishEventHeaderImpl(const GEventDataCollectionHeader *gheader) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

    // get or instantiate root tree from the map
    GRootTree *headerTree = getOrInstantiateHeaderTree(gheader);

    // fill variables and tree
    return headerTree->fillTree(gheader);
}
