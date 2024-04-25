// gstreamer
#include "gstreamerTEXTFactory.h"

bool GstreamerTextFactory::openConnection()
{
	ofile = new ofstream(outputFileName);
	
	// PRAGMA: need to check if file opened successfully
	return true;
}

bool GstreamerTextFactory::closeConnection()
{
	ofile->close();
	delete ofile;

	
	// PRAGMA: need to check if file closed successfully
	return true;
}



