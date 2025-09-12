// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
bool GstreamerTextFactory::publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gheader) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Header Bank {\n";
	ofile << GTABTAB << " time: " << gheader->getTimeStamp() << "\n";
	ofile << GTABTAB << " thread id: " << gheader->getThreadID() << "\n";
	ofile << GTAB << "}\n";

	return true;
}
