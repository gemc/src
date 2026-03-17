// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerTextFactory::startRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	auto runId = run_data->getHeader()->getRunID();
	ofile << "Run number " << runId << " {\n";
	ofile << GTAB << "Number of events: " <<  run_data->getHeader()->get_events_processed() << "\n";
	ofile << GTAB << "Number of events with payload: " <<  run_data->getHeader()->get_events_with_payload() << "\n";

	return true;
}

bool GstreamerTextFactory::endRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	auto runId = run_data->getHeader()->getRunID();

	ofile << "} end of run " << runId << "\n";

	return true;
}
