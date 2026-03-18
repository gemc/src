// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Cache run-level context needed by later CSV publish routines.

bool GstreamerCsvFactory::startRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {

	if (!ofile_digitized.is_open()) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename_digitized());
	}

	// Cache the run identifier for diagnostics and later row generation.
	runId = run_data->getHeader()->getRunID();

	return true;
}


bool GstreamerCsvFactory::endRunImpl([[maybe_unused]] const std::shared_ptr<GRunDataCollection>& run_data) {
	return true;
}