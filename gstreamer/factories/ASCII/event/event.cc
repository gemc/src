// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	auto eventNumber = event_data->getHeader()->getG4LocalEvn();
	*ofile << "Event n. " << eventNumber << " {" << std::endl;

	return true;
}


bool GstreamerTextFactory::endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	int eventNumber = event_data->getHeader()->getG4LocalEvn();

	*ofile << "} end of event " << eventNumber << std::endl;

	return true;
}
