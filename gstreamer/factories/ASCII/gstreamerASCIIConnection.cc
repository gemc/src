// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::openConnection() {
	ofile = new std::ofstream(filename());

	if (!ofile->is_open()) { log->error(ERR_CANTOPENOUTPUT, "GstreamerTextFactory: could not open file " + filename()); }

	log->info(0, "GstreamerTextFactory: opened file " + filename());
	return true;
}

bool GstreamerTextFactory::closeConnectionImpl() {

	flushEventBuffer();

	if (ofile->is_open()) { ofile->close(); }

	if (ofile->is_open()) { log->error(ERR_CANTCLOSEOUTPUT, "GstreamerTextFactory: could not close file " + filename()); }

///	delete ofile;
	return true;
}
