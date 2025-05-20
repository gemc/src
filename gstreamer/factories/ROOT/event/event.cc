// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::startEvent(const GEventDataCollection* eventData, const std::shared_ptr<GLogger>& log) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	log->info(2, "Start of event ", eventData->getHeader()->getG4LocalEvn(),
	          " in ", gstreamer_definitions.name, ": initializing trees");

	for (auto t : (*gRootTrees)) { t.second->initTreeForTheEvent(); }

	return true;
}


bool GstreamerRootFactory::endEvent(const GEventDataCollection* eventData, const std::shared_ptr<GLogger>& log) {
	log->info(2, "End of event ", eventData->getHeader()->getG4LocalEvn(), " in ", gstreamer_definitions.name);

	return true;
}
