// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerTextFactory::publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Header Bank {\n";
	ofile << GTABTAB << " time: " << gevent_header->getTimeStamp() << "\n";
	ofile << GTABTAB << " thread id: " << gevent_header->getThreadID() << "\n";
	ofile << GTAB << "}\n";

	return true;
}
