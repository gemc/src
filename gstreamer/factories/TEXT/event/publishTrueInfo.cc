// gstreamer
#include "../gstreamerTEXTFactory.h"

bool GstreamerTextFactory::publishEventTrueInfoData(const string detectorName, const vector<GTrueInfoData*>* trueInfoData) {

	if(ofile == nullptr) return false;
	
	*ofile << GTAB << "Detector <" <<  detectorName << "> True Info Bank {" << endl;

	for ( auto trueInfoHit: *trueInfoData ) {

		string identifierString = "";
		vector<GIdentifier> gidentity = trueInfoHit->getIdentity();
		for ( size_t i=0; i<gidentity.size() - 1; i++ ) {
			identifierString += gidentity[i].getName() + "->" + to_string(gidentity[i].getValue()) + ", ";
		}
		identifierString += gidentity.back().getName() + "->" + to_string(gidentity.back().getValue()) ;

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
