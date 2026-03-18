// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Fill one detector true-information ROOT tree for the current event.

bool GstreamerRootFactory::publishEventTrueInfoDataImpl(const std::string&                       detectorName,
														const std::vector<const GTrueInfoData*>& trueInfoData) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	if (!trueInfoData.empty()) {
		const auto& trueInforDataTree = getOrInstantiateTrueInfoDataTree(detectorName, trueInfoData.front());

		return trueInforDataTree->fillTree(trueInfoData);
	}

	return false;
}