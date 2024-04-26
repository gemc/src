// gstreamer
#include "../gstreamerROOTFactory.h"

bool GstreamerRootFactory::publishEventTrueInfoData(const string detectorName, const vector<GTrueInfoData *> *trueInfoData) {

    if (rootfile == nullptr) return false;

    // get or instantiate root tree from the map
    if (trueInfoData->size()) {
        GRootTree *trueInforDataTree = getOrInstantiateTrueInfoDataTree(detectorName, trueInfoData->front());

        return trueInforDataTree->fillTree(trueInfoData);
    }

    return false;
}
