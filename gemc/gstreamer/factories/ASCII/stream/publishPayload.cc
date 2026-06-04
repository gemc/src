// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Serialize the frame payload as a simple list of payload rows.

bool GstreamerTextFactory::publishPayloadImpl(const std::vector<GIntegralPayload*>* payload) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Payload  {\n";

	// Each GIntegralPayload contributes one line made of its packed integer values.
	for (auto pload : *payload) {
		for (auto pvar : pload->getPayload()) { ofile << GTABTAB << pvar; }
		ofile << "\n";
	}
	ofile << GTAB << "}\n";

	return true;
}