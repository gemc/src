// gstreamer
#include "gstreamerJLABSROFactory.h"
#include "gstreamerConventions.h"

bool GstreamerJSROFactory::openConnection(const std::shared_ptr<GLogger>& log)
{
	ofile = new std::ofstream(gstreamer_definitions.name);
	
	if (!ofile->is_open()) { log->error(ERR_CANTOPENOUTPUT, "GstreamerJSROFactory: could not open file " + gstreamer_definitions.name); }

	log->info(0, "GstreamerJSROFactory: opened file " + gstreamer_definitions.name);
	return true;
}

bool GstreamerJSROFactory::closeConnection(const std::shared_ptr<GLogger>& log)
{
	ofile->close();
	if (!ofile->is_open()) { log->error(ERR_CANTOPENOUTPUT, "GstreamerJSROFactory: could not close file " + gstreamer_definitions.name); }

	delete ofile;
	return true;
}
