// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::publishEventDigitizedData(const string                   detectorName,
                                                     const vector<GDigitizedData*>* digitizedData,
                                                     std::shared_ptr<GLogger>&      log) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	*ofile << GTAB << "Detector <" << detectorName << "> Digitized Bank {" << std::endl;

	for (auto dgtzHit : *digitizedData) {
		string identifierString = dgtzHit->getIdentityString();

		*ofile << GTABTAB << "Hit address: " << identifierString << " {" << std::endl;

		// argument passed to getter: 0 = do not get sro vars
		for (const auto& [variableName, value] : dgtzHit->getIntObservablesMap(0)) { *ofile << GTABTABTAB << variableName << ": " << value << std::endl; }
		for (const auto& [variableName, value] : dgtzHit->getDblObservablesMap(0)) { *ofile << GTABTABTAB << variableName << ": " << value << std::endl; }

		*ofile << GTABTAB << "}" << std::endl;
	}
	*ofile << GTAB << "}" << std::endl;

	return true;
}
