// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Manage the lifetime of the text output stream used by the ASCII plugin.

bool GstreamerTextFactory::openConnection() {
	if (ofile.is_open()) {
		// Already open for this streamer instance.
		return true;
	}

	ofile.clear();

	// Open for writing and truncate any existing file content so each run starts fresh.
	ofile.open(filename(), std::ios::out | std::ios::trunc);

	if (!ofile.is_open() || !ofile) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename());
	}

	log->info(1, SFUNCTION_NAME, "GstreamerTextFactory: opened file " + filename());

	return true;
}

bool GstreamerTextFactory::closeConnectionImpl() {
	// The public closeConnection() wrapper already flushes pending events before calling this method.

	if (ofile.is_open()) ofile.close();
	if (ofile.is_open()) { log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename()); }

	log->info(1, SFUNCTION_NAME, "GstreamerTextFactory: closed file " + filename());

	return true;
}