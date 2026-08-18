// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Serialize the event header bank for the ASCII backend.
// Use '\n' instead of std::endl so each line does not force a flush.

bool GstreamerTextFactory::publishEventHeaderImpl([[maybe_unused]] const std::unique_ptr<GEventHeader>& gevent_header) {
	if (!ofile.is_open()) {
		log->error(gstreamer::ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename());
	}

	ofile << guts::GTAB << "Header Bank {\n";
	ofile << guts::GTABTAB << " time: " << gevent_header->getTimeStamp() << "\n";
	ofile << guts::GTABTAB << " thread id: " << gevent_header->getThreadID() << "\n";
	ofile << guts::GTAB << "}\n";

	return true;
}