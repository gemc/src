// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::publishEventHeaderImpl([[maybe_unused]]const std::unique_ptr<GEventHeader>& gheader)
{
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }
	
	*ofile << GTAB << "Header Bank {" << std::endl;
	*ofile << GTABTAB << " time: " << gheader->getTimeStamp() << std::endl;
	*ofile << GTABTAB << " g4run-local event number: " << gheader->getG4LocalEvn() << std::endl;
	*ofile << GTABTAB << " thread id: " << gheader->getThreadID() << std::endl;
	*ofile << GTAB << "}" << std::endl;

	return true;
}
