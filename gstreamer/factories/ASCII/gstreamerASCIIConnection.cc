// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::openConnectionImpl() {
	ofile = new std::ofstream(gstreamer_definitions.name);

	if (!ofile->is_open()) { log->error(ERR_CANTOPENOUTPUT, "GstreamerTextFactory: could not open file " + gstreamer_definitions.name); }

	log->info(0, "GstreamerTextFactory: opened file " + gstreamer_definitions.name);
	return true;
}

bool GstreamerTextFactory::closeConnectionImpl() {
	ofile->close();

	// check if file is closed
	if (ofile->is_open()) { log->error(ERR_CANTCLOSEOUTPUT, "GstreamerTextFactory: could not close file " + gstreamer_definitions.name); }

	delete ofile;
	return true;
}
