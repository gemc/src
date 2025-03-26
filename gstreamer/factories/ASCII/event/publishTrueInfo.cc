// gstreamer
#include "../gstreamerASCIIFactory.h"

bool GstreamerTextFactory::publishEventTrueInfoData(const string detectorName, const vector<GTrueInfoData*>* trueInfoData) {

	if(ofile == nullptr) return false;
	
	*ofile << GTAB << "Detector <" <<  detectorName << "> True Info Bank {" << endl;

	for ( auto trueInfoHit: *trueInfoData ) {

		string identifierString = trueInfoHit->getIdentityString();

		*ofile << GTABTAB << "Hit address: " << identifierString << " {" << endl;

		for ( auto [variableName, value]: trueInfoHit->getFloatVariablesMap() ) {
			*ofile << GTABTABTAB << variableName << ": " << value << endl;
		}
		for ( auto [variableName, value]: trueInfoHit->getStringVariablesMap() ) {
			*ofile << GTABTABTAB << variableName << ": " << value << endl;
		}

		*ofile << GTABTAB << "}" << endl;


	}
	*ofile << GTAB << "}" << endl;


	return true;
}
