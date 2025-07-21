// gstreamer
#include "../gstreamerROOTFactory.h"

bool GstreamerRootFactory::startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {

	log->info(2, "Start of event ", event_data->getHeader()->getG4LocalEvn()," in ", filename());

	return true;
}


bool GstreamerRootFactory::endEventImpl([[maybe_unused]]const std::shared_ptr<GEventDataCollection>& event_data) {

	log->info(2, "End of event ", event_data->getHeader()->getG4LocalEvn(), " in ", filename());

	return true;
}
