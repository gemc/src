// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::publishPayloadImpl([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	return true;
}
