// gstreamer
#include "../gstreamerROOTFactory.h"

bool GstreamerRootFactory::startEventImpl([[maybe_unused]]  const std::unique_ptr<GEventDataCollection>& eventData) {

	log->info(2, "Start of event ", eventData->getHeader()->getG4LocalEvn(),
	          " in ", filename(), ": initializing trees");

	return true;
}


bool GstreamerRootFactory::endEventImpl([[maybe_unused]]  const std::unique_ptr<GEventDataCollection>& eventData) {

	log->info(2, "End of event ", eventData->getHeader()->getG4LocalEvn(), " in ", filename());

	return true;
}
