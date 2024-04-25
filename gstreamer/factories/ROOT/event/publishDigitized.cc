// gstreamer
#include "../gstreamerROOTFactory.h"

bool GstreamerRootFactory::publishEventDigitizedData(const string detectorName, const vector<GDigitizedData *> *digitizedData) {

    if (rootfile == nullptr) return false;

    // get or instantiate root tree from the map
    if (digitizedData->size()) {
        GRootTree *digitizedDataTree = getOrInstantiateDigitizedDataTree(detectorName, digitizedData->front());

        return digitizedDataTree->fillTree(digitizedData);
    }

    return false;
}
