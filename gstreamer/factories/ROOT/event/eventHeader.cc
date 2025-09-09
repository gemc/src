// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"


bool GstreamerRootFactory::publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gheader) {

	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

    // get or instantiate the root tree from the map
	const auto& headerTree = getOrInstantiateHeaderTree(gheader);

	// fill variables and tree
    return headerTree->fillTree(gheader);
}
