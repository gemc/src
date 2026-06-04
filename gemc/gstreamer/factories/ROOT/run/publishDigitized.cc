// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Fill the run-level digitized ROOT tree for one detector.

bool GstreamerRootFactory::publishRunDigitizedDataImpl(const std::string&                        detectorName,
													   const std::vector<const GDigitizedData*>& digitizedData) {
	if (rootfile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is not initialized"); }

	// Lazily create the detector tree from the first hit, then fill it with the full collection.
	if (!digitizedData.empty()) {
		const auto& digitizedDataTree = getOrInstantiateDigitizedDataTree(detectorName, digitizedData.front());

		return digitizedDataTree->fillTree(digitizedData);
	}

	return false;
}