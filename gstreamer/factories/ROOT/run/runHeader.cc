// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Non-Doxygen implementation file: behavior is documented in the header.

bool GstreamerRootFactory::publishRunHeaderImpl([[maybe_unused]] const std::unique_ptr<GRunHeader>& grun_header) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	// get or instantiate the root tree from the map
	const auto& headerTree = getOrInstantiateHeaderTree(grun_header);

	// fill variables and tree
	return headerTree->fillTree(grun_header);
}
