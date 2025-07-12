// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::startEventImpl(const GEventDataCollection* eventData) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	log->info(2, "Start of event ", eventData->getHeader()->getG4LocalEvn(),
	          " in ", filename(), ": initializing trees");

	for (auto t : (*gRootTrees)) { t.second->initTreeForTheEvent(); }

	return true;
}


bool GstreamerRootFactory::endEventImpl(const GEventDataCollection* eventData) {
	log->info(2, "End of event ", eventData->getHeader()->getG4LocalEvn(), " in ", filename());

	return true;
}
