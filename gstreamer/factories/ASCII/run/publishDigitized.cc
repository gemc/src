// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Write one run-level detector digitized bank in a readable nested text form.
// Use '\n' instead of std::endl so each line does not force a flush.

bool GstreamerTextFactory::publishRunDigitizedDataImpl(const std::string&                        detectorName,
													   const std::vector<const GDigitizedData*>& digitizedData)
{
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Detector <" << detectorName << "> Digitized Bank {\n";

	for (auto dgtzHit : digitizedData) {
		std::string identifierString = dgtzHit->getIdentityString();

		ofile << GTABTAB << "Hit address: " << identifierString << " {\n";

		// Argument passed to getter:
		// 0 means "do not include SRO variables".
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