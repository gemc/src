// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::publishEventTrueInfoData(const string                  detectorName,
                                                    const vector<GTrueInfoData*>* trueInfoData,
                                                    std::shared_ptr<GLogger>&     log) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	*ofile << GTAB << "Detector <" << detectorName << "> True Info Bank {" << std::endl;

	for (auto trueInfoHit : *trueInfoData) {
		string identifierString = trueInfoHit->getIdentityString();

		*ofile << GTABTAB << "Hit address: " << identifierString << " {" << std::endl;

		for (const auto& [variableName, value] : trueInfoHit->getDoubleVariablesMap()) { *ofile << GTABTABTAB << variableName << ": " << value << std::endl; }
		for (const auto& [variableName, value] : trueInfoHit->getStringVariablesMap()) { *ofile << GTABTABTAB << variableName << ": " << value << std::endl; }

		*ofile << GTABTAB << "}" << std::endl;
	}
	*ofile << GTAB << "}" << std::endl;


	return true;
}
