// gstreamer
#include "../gstreamerROOTFactory.h"

bool GstreamerRootFactory::startEventImpl(const GEventDataCollection* eventData) {


	log->info(2, "Start of event ", eventData->getHeader()->getG4LocalEvn(),
	          " in ", filename(), ": initializing trees");

	return true;
}


bool GstreamerRootFactory::endEventImpl(const GEventDataCollection* eventData) {

	log->info(2, "End of event ", eventData->getHeader()->getG4LocalEvn(), " in ", filename());

	return true;
}
