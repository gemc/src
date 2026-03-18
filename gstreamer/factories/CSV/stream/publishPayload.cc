// gstreamer
#include "gstreamerCSVFactory.h"

// Implementation summary:
// Frame-payload publication is currently a no-op for the CSV backend.

bool GstreamerCsvFactory::publishPayloadImpl([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) {
	return true;
}