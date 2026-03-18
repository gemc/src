// gstreamer

#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerCsvFactory::startRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {

	if (!ofile_digitized.is_open()) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename_digitized());
	}

	// Cache the run id for later row emission in the publish routines.
	runId = run_data->getHeader()->getRunID();

	return true;
}


bool GstreamerCsvFactory::endRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
	return true;
}
