// gstreamer
#include "../gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::publishEventTrueInfoData(const string                    detectorName,
                                                    const vector<GTrueInfoData*>*   trueInfoData,
                                                    std::shared_ptr<GLogger>& log) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	// get or instantiate root tree from the map
	if (trueInfoData->size()) {
		GRootTree* trueInforDataTree = getOrInstantiateTrueInfoDataTree(detectorName, trueInfoData->front(), log);

		return trueInforDataTree->fillTree(trueInfoData);
	}

	return false;
}
