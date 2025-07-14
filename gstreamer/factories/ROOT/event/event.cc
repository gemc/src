// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::startEventImpl(const GEventDataCollection* eventData) {

	if (!eventData || !eventData->getHeader()) {
		log->error(ERR_PUBLISH_ERROR, "eventData or event header is null.");
	}

	log->info(2, "Start of event ", eventData->getHeader()->getG4LocalEvn(),
	          " in ", filename(), ": initializing trees");

	return true;
}


bool GstreamerRootFactory::endEventImpl(const GEventDataCollection* eventData) {

	log->info(2, "End of event ", eventData->getHeader()->getG4LocalEvn(), " in ", filename());

	return true;
}
