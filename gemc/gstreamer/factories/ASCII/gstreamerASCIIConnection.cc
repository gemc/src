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
		log->error(gstreamer::ERR_CANTOPENOUTPUT, "%1 could not open file %2", SFUNCTION_NAME, filename());
	}

	log->info(1, "%1GstreamerTextFactory: opened file %2", SFUNCTION_NAME, filename());

	return true;
}

bool GstreamerTextFactory::closeConnectionImpl() {
	// The public closeConnection() wrapper already flushes pending events before calling this method.

	if (ofile.is_open()) ofile.close();
	if (ofile.is_open()) {
		log->error(gstreamer::ERR_CANTCLOSEOUTPUT, "%1 could not close file %2", SFUNCTION_NAME, filename());
	}

	log->info(1, "%1GstreamerTextFactory: closed file %2", SFUNCTION_NAME, filename());

	return true;
}