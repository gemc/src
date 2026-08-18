// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Write one detector digitized bank for the current event in human-readable text form.
// Use '\n' instead of std::endl so each line does not force a flush.

bool GstreamerTextFactory::publishEventDigitizedDataImpl(const std::string&                        detectorName,
														 const std::vector<const GDigitizedData*>& digitizedData) {
	if (!ofile.is_open()) {
		log->error(gstreamer::ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename());
	}

	ofile << guts::GTAB << "Detector <" << detectorName << "> Digitized Bank {\n";

	for (auto dgtzHit : digitizedData) {
		auto identifierString = getIdentityString(dgtzHit->getIdentity());

		ofile << guts::GTABTAB << "Hit address: " << identifierString << " {\n";

		// Argument passed to getter:
		// 0 means "do not include SRO variables".
		for (const auto& [variableName, value] : dgtzHit->getIntObservablesMap(0)) {
			ofile << guts::GTABTABTAB << variableName << ": " << value << "\n";
		}
		for (const auto& [variableName, value] : dgtzHit->getDblObservablesMap(0)) {
			ofile << guts::GTABTABTAB << variableName << ": " << value << "\n";
		}

		ofile << guts::GTABTAB << "}\n";
	}
	ofile << guts::GTAB << "}\n";

	return true;
}