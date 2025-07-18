// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// bool GstreamerTextFactory::publishEventTrueInfoDataImpl(const std::string                  detectorName,
//                                                         const std::vector<GTrueInfoData*>* trueInfoData) {
// 	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }
//
// 	*ofile << GTAB << "Detector <" << detectorName << "> True Info Bank {" << std::endl;
//
// 	for (auto trueInfoHit : *trueInfoData) {
// 		std::string identifierString = trueInfoHit->getIdentityString();
//
// 		*ofile << GTABTAB << "Hit address: " << identifierString << " {" << std::endl;
//
// 		for (const auto& [variableName, value] : trueInfoHit->getDoubleVariablesMap()) { *ofile << GTABTABTAB << variableName << ": " << value << std::endl; }
// 		for (const auto& [variableName, value] : trueInfoHit->getStringVariablesMap()) { *ofile << GTABTABTAB << variableName << ": " << value << std::endl; }
//
// 		*ofile << GTABTAB << "}" << std::endl;
// 	}
// 	*ofile << GTAB << "}" << std::endl;
//
//
// 	return true;
// }
