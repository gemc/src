// gstreamer
#include "gstreamerCSVFactory.h"

// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerCsvFactory::publishPayloadImpl([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) {
	return true;
}
