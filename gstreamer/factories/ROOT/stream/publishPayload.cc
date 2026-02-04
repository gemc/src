// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerRootFactory::publishPayloadImpl([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	return true;
}
