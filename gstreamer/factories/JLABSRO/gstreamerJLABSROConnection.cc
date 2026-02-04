// gstreamer
#include "gstreamerJLABSROFactory.h"
#include "gstreamerConventions.h"

// Non-Doxygen implementation file: behavior is documented in the header.

bool GstreamerJSROFactory::openConnection() {
	ofile = new std::ofstream(filename());
	if (!ofile->is_open()) {
		log->error(ERR_CANTOPENOUTPUT, "GstreamerJSROFactory: could not open file " + filename());
	}

	log->info(0, "GstreamerJSROFactory: opened file " + filename());
	return true;
}

bool GstreamerJSROFactory::closeConnectionImpl() {
	if (ofile->is_open()) { ofile->close(); }

	if (ofile->is_open()) {
		log->error(ERR_CANTOPENOUTPUT, "GstreamerJSROFactory: could not close file " + filename());
	}

	delete ofile;
	return true;
}
