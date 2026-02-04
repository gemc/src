// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerRootFactory::publishEventTrueInfoDataImpl(const std::string&                       detectorName,
                                                        const std::vector<const GTrueInfoData*>& trueInfoData) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	// get or instantiate the root tree from the map
	if (!trueInfoData.empty()) {
		const auto& trueInforDataTree = getOrInstantiateTrueInfoDataTree(detectorName, trueInfoData.front());

		return trueInforDataTree->fillTree(trueInfoData);
	}

	return false;
}
