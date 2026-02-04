// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// Non-Doxygen implementation file: behavior is documented in the header.

bool GstreamerCsvFactory::openConnection() {
	// Both streams must be open for the CSV plugin to operate.
	if (ofile_true_info.is_open() && ofile_true_info.is_open()) {
		// Already open for this thread; nothing to do.
		return true;
	}

	if (!ofile_true_info.is_open()) {
		ofile_true_info.clear(); // clear fail/eof bits from last use
		// std::ios::out — open for writing.
		// std::ios::trunc — if the file already exists, truncate it to size 0 on open (wipe its contents). If it doesn’t exist, it will be created.
		// another variant: std::ios::app: append
		ofile_true_info.open(filename_true_info(), std::ios::out | std::ios::trunc);

		if (!ofile_true_info.is_open() || !ofile_true_info) {
			log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename_true_info());
		}

		log->info(0, "GstreamerCsvFactory: opened file " + filename_true_info());
	}

	if (!ofile_digitized.is_open()) {
		ofile_digitized.clear(); // clear fail/eof bits from last use
		// std::ios::out — open for writing.
		// std::ios::trunc — if the file already exists, truncate it to size 0 on open (wipe its contents). If it doesn’t exist, it will be created.
		// another variant: std::ios::app: append
		ofile_digitized.open(filename_digitized(), std::ios::out | std::ios::trunc);

		if (!ofile_digitized.is_open() || !ofile_true_info) {
			log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename_digitized());
		}

		log->info(0, "GstreamerCsvFactory: opened file " + filename_digitized());
	}


	return true;
}

bool GstreamerCsvFactory::closeConnectionImpl() {
	// Ensure any buffered events are written before closing the files.
	flushEventBuffer();

	if (ofile_true_info.is_open()) ofile_true_info.close();
	if (ofile_digitized.is_open()) ofile_digitized.close();

	if (ofile_true_info.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename_true_info());
	}
	if (ofile_digitized.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename_digitized());
	}

	return true;
}
