// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerRootFactory::publishEventDigitizedDataImpl(const std::string&                        detectorName,
                                                         const std::vector<const GDigitizedData*>& digitizedData) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	// get or instantiate the root tree from the map
	if (!digitizedData.empty()) {
		const auto& digitizedDataTree = getOrInstantiateDigitizedDataTree(detectorName, digitizedData.front());

		return digitizedDataTree->fillTree(digitizedData);
	}

	return false;
}
