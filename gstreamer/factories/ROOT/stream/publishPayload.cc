// gstreamer
#include "../gstreamerROOTFactory.h"

bool GstreamerRootFactory::publishPayload([[maybe_unused]] const vector<GIntegralPayload *> *payload) {
    if (rootfile == nullptr) return false;

    return true;
}
