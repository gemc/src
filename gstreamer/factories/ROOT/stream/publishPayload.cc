// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::publishPayload([[maybe_unused]] const std::vector<GIntegralPayload *> *payload, const std::shared_ptr<GLogger>& log) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

    return true;
}
