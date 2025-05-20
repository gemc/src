// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::startEvent(const GEventDataCollection* eventData, const std::shared_ptr<GLogger>& log) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	auto eventNumber = eventData->getHeader()->getG4LocalEvn();
	*ofile << "Event n. " << eventNumber << " {" << std::endl;

	return true;
}


bool GstreamerTextFactory::endEvent(const GEventDataCollection* eventData, const std::shared_ptr<GLogger>& log) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	int eventNumber = eventData->getHeader()->getG4LocalEvn();

	*ofile << "} end of event " << eventNumber << std::endl;

	return true;
}
