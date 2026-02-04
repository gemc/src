// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerTextFactory::publishEventDigitizedDataImpl(const std::string&                        detectorName,
                                                         const std::vector<const GDigitizedData*>& digitizedData) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Detector <" << detectorName << "> Digitized Bank {\n";

	for (auto dgtzHit : digitizedData) {
		std::string identifierString = dgtzHit->getIdentityString();

		ofile << GTABTAB << "Hit address: " << identifierString << " {\n";

		// argument passed to getter: 0 = do not get sro vars
		for (const auto& [variableName, value] : dgtzHit->getIntObservablesMap(0)) {
			ofile << GTABTABTAB << variableName << ": " << value << "\n";
		}
		for (const auto& [variableName, value] : dgtzHit->getDblObservablesMap(0)) {
			ofile << GTABTABTAB << variableName << ": " << value << "\n";
		}

		ofile << GTABTAB << "}\n";
	}
	ofile << GTAB << "}\n";

	return true;
}
