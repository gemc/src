// gstreamer
#include "gstreamerROOTFactory.h"

// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerRootFactory::startRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
	log->info(2, "Start of run ", run_data->getHeader()->getRunID(), " in ", filename());

	return true;
}


bool GstreamerRootFactory::endRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
	log->info(2, "End of run ", run_data->getHeader()->getRunID(), " in ", filename());

	return true;
}
