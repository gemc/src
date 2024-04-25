// gstreamer
#include "../gstreamerTEXTFactory.h"

bool GstreamerTextFactory::publishEventDigitizedData(const string detectorName, const vector<GDigitizedData*>* digitizedData) {

	if(ofile == nullptr) return false;

	*ofile << GTAB << "Detector <" <<  detectorName << "> Digitized Bank {" << endl;

	for ( auto dgtzHit: *digitizedData ) {

		string identifierString = "";
		vector<GIdentifier> gidentity = dgtzHit->getIdentity();
		for ( size_t i=0; i<gidentity.size() - 1; i++ ) {
			identifierString += gidentity[i].getName() + "->" + to_string(gidentity[i].getValue()) + ", ";
		}
		identifierString += gidentity.back().getName() + "->" + to_string(gidentity.back().getValue()) ;

		*ofile << GTABTAB << "Hit address: " << identifierString << " {" << endl;

		// argument passed to getter: 0 = do not get sro vars
		for ( auto [variableName, value]: dgtzHit->getIntObservablesMap(0)  ) {
			*ofile << GTABTABTAB << variableName << ": " << value << endl;
		}
		for ( auto [variableName, value]: dgtzHit->getFltObservablesMap(0) ) {
			*ofile << GTABTABTAB << variableName << ": " << value << endl;
		}

		*ofile << GTABTAB << "}" << endl;
	}
	*ofile << GTAB << "}" << endl;
	
	return true;
}
