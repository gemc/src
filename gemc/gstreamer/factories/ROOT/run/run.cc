// gstreamer
#include "gstreamerROOTFactory.h"

// Implementation summary:
// Emit lightweight lifecycle logs around run publication for the ROOT backend.

bool GstreamerRootFactory::startRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
	log->info(2, "Start of run %1 in %2", run_data->getHeader()->getRunID(), filename());

	return true;
}


bool GstreamerRootFactory::endRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
	log->info(2, "End of run %1 in %2", run_data->getHeader()->getRunID(), filename());

	return true;
}