// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"


bool GstreamerRootFactory::publishEventHeader(const GEventDataCollectionHeader *gheader, std::shared_ptr<GLogger>& log) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

    // get or instantiate root tree from the map
    GRootTree *headerTree = getOrInstantiateHeaderTree(gheader, log);

    // fill variables and tree
    return headerTree->fillTree(gheader);
}
