// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Fill the run-header ROOT tree.

bool GstreamerRootFactory::publishRunHeaderImpl([[maybe_unused]] const std::unique_ptr<GRunHeader>& grun_header) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	// Retrieve the unique run-header tree and fill it with the current header values.
	const auto& headerTree = getOrInstantiateHeaderTree(grun_header);

	return headerTree->fillTree(grun_header);
}