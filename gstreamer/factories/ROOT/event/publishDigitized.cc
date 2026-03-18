// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Fill one detector digitized ROOT tree for the current event.

bool GstreamerRootFactory::publishEventDigitizedDataImpl(const std::string&                        detectorName,
														 const std::vector<const GDigitizedData*>& digitizedData) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	if (!digitizedData.empty()) {
		const auto& digitizedDataTree = getOrInstantiateDigitizedDataTree(detectorName, digitizedData.front());

		return digitizedDataTree->fillTree(digitizedData);
	}

	return false;
}