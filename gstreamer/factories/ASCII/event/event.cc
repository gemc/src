// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Write the opening and closing delimiters for one event block.
// Use '\n' instead of std::endl so each line does not force a flush.

bool GstreamerTextFactory::startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	auto eventNumber = event_data->getHeader()->getG4LocalEvn();
	ofile << "Event n. " << eventNumber << " {\n";

	return true;
}


bool GstreamerTextFactory::endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	int eventNumber = event_data->getHeader()->getG4LocalEvn();

	ofile << "} end of event " << eventNumber << "\n";

	return true;
}