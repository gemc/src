// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::publishEventDigitizedData(const string detectorName, const vector<GDigitizedData *> *digitizedData, std::shared_ptr<GLogger>& log) {

	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

    // get or instantiate root tree from the map
    if (digitizedData->size()) {
        GRootTree *digitizedDataTree = getOrInstantiateDigitizedDataTree(detectorName, digitizedData->front(), log);

        return digitizedDataTree->fillTree(digitizedData);
    }

    return false;
}
