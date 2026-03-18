// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Manage the lifetime of the two CSV streams used by the plugin.

bool GstreamerCsvFactory::openConnection() {
	// Both streams must be open for the CSV backend to operate correctly.
	if (ofile_true_info.is_open() && ofile_digitized.is_open()) {
		return true;
	}

	if (!ofile_true_info.is_open()) {
		ofile_true_info.clear();
		ofile_true_info.open(filename_true_info(), std::ios::out | std::ios::trunc);

		if (!ofile_true_info.is_open() || !ofile_true_info) {
			log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename_true_info());
		}

		log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: opened file " + filename_true_info());
	}

	if (!ofile_digitized.is_open()) {
		ofile_digitized.clear();
		ofile_digitized.open(filename_digitized(), std::ios::out | std::ios::trunc);

		if (!ofile_digitized.is_open() || !ofile_digitized) {
			log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename_digitized());
		}

		log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: opened file " + filename_digitized());
	}

	return true;
}

bool GstreamerCsvFactory::closeConnectionImpl() {
	// The public closeConnection() wrapper already flushes buffered events before this method runs.

	if (ofile_true_info.is_open()) ofile_true_info.close();
	if (ofile_digitized.is_open()) ofile_digitized.close();

	if (ofile_true_info.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename_true_info());
	}
	if (ofile_digitized.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename_digitized());
	}

	log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: closed file " + filename_true_info());
	log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: closed file " + filename_digitized());

	return true;
}