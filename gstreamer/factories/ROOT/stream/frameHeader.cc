// gstreamer
#include "../gstreamerROOTFactory.h"

bool GstreamerRootFactory::publishFrameHeader([[maybe_unused]] const GFrameDataCollectionHeader *gframeHeader) {
    if (rootfile == nullptr) return false;

    return true;
}
