// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Cache per-event state needed by later CSV publish routines.

bool GstreamerCsvFactory::startEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
	if (!ofile_true_info.is_open()) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename_true_info());
	}
	if (!ofile_digitized.is_open()) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename_digitized());
	}

	// Cache the event number for the detector rows written later in the publish sequence.
	event_number = event_data->getHeader()->getG4LocalEvn();

	return true;
}


bool GstreamerCsvFactory::endEventImpl([[maybe_unused]] const std::shared_ptr<GEventDataCollection>& event_data) {
	return true;
}