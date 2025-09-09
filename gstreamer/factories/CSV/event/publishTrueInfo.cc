// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
bool GstreamerTextFactory::publishEventTrueInfoDataImpl(const std::string& detectorName, const std::vector<const GTrueInfoData*>& trueInfoData) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Detector <" << detectorName << "> True Info Bank {\n";

	for (auto trueInfoHit : trueInfoData) {
		std::string identifierString = trueInfoHit->getIdentityString();

		ofile << GTABTAB << "Hit address: " << identifierString << " {\n";

		for (const auto& [variableName, value] : trueInfoHit->getDoubleVariablesMap()) { ofile << GTABTABTAB << variableName << ": " << value << "\n"; }
		for (const auto& [variableName, value] : trueInfoHit->getStringVariablesMap()) { ofile << GTABTABTAB << variableName << ": " << value << "\n"; }

		ofile << GTABTAB << "}\n";
	}
	ofile << GTAB << "}\n";

	return true;
}
