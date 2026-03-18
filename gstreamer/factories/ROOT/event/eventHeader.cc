// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Fill the event-header ROOT tree.

bool GstreamerRootFactory::publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	const auto& headerTree = getOrInstantiateHeaderTree(gevent_header);

	return headerTree->fillTree(gevent_header);
}