// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::publishEventDigitizedDataImpl(const std::string detectorName, const std::vector<GDigitizedData *> *digitizedData) {

	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

    // get or instantiate root tree from the map
    if (digitizedData->size()) {
        GRootTree *digitizedDataTree = getOrInstantiateDigitizedDataTree(detectorName, digitizedData->front());

        return digitizedDataTree->fillTree(digitizedData);
    }

    return false;
}
