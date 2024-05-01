// gstreamer
#include "../gstreamerROOTFactory.h"

bool GstreamerRootFactory::startStream([[maybe_unused]] const GFrameDataCollection *frameRunData) {
    if (rootfile == nullptr) return false;

    return true;
}


bool GstreamerRootFactory::endStream([[maybe_unused]] const GFrameDataCollection *frameRunData) {
    if (rootfile == nullptr) return false;

    return true;
}
