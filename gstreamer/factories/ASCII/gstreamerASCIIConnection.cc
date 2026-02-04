// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation note (non-Doxygen):
// Header documentation in gstreamerASCIIFactory.h is authoritative.

bool GstreamerTextFactory::openConnection() {
	if (ofile.is_open()) {
		// Already open for this thread; nothing to do.
		return true;
	}

	ofile.clear(); // clear fail/eof bits from last use
	// std::ios::out — open for writing.
	// std::ios::trunc — if the file already exists, truncate it to size 0 on open (wipe its contents). If it doesn’t exist, it will be created.
	// another variant: std::ios::app: append
	ofile.open(filename(), std::ios::out | std::ios::trunc);

	if (!ofile.is_open() || !ofile) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename());
	}

	log->info(0, "GstreamerTextFactory: opened file " + filename());
	return true;
}

bool GstreamerTextFactory::closeConnectionImpl() {
	// Ensure any buffered events are written before closing the file.
	flushEventBuffer();

	if (ofile.is_open()) ofile.close();

	if (ofile.is_open()) { log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename()); }

	return true;
}
