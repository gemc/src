// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::publishEventTrueInfoDataImpl(const std::string                    detectorName,
                                                    const std::vector<GTrueInfoData*>*   trueInfoData) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	// get or instantiate root tree from the map
	if (trueInfoData->size()) {
		GRootTree* trueInforDataTree = getOrInstantiateTrueInfoDataTree(detectorName, trueInfoData->front());

		return trueInforDataTree->fillTree(trueInfoData);
	}

	return false;
}
