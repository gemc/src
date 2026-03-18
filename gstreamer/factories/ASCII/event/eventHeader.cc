// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Serialize the event header bank for the ASCII backend.
// Use '\n' instead of std::endl so each line does not force a flush.

bool GstreamerTextFactory::publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Header Bank {\n";
	ofile << GTABTAB << " time: " << gevent_header->getTimeStamp() << "\n";
	ofile << GTABTAB << " thread id: " << gevent_header->getThreadID() << "\n";
	ofile << GTAB << "}\n";

	return true;
}