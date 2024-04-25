// gtouchable
#include "gtouchable.h"
#include "gtouchableConventions.h"

// glibrary
#include "gutilities.h"
#include "gutsConventions.h"
using namespace std;


// constructor from digitization and gidentity strings
// called in GDetectorConstruction::ConstructSDandField
// to register a new gtouchable in the sensitive detector gtouchable map
GTouchable::GTouchable(string digitization, string gidentityString, vector<double> dimensions, bool verb) :
verbosity(verb),
trackId(0),
eMultiplier(1),
stepTimeAtElectronicsIndex(GTOUCHABLEUNSETTIMEINDEX),
detectorDimensions(dimensions) {

	// gtype from digitization
	if ( digitization == FLUXNAME ) {
		gType = flux;
	} else if ( digitization == COUNTERNAME ) {
		gType = particleCounter;
	} else if ( digitization == DOSIMETERNAME ) {
		gType = dosimeter;
	} else {
		gType = readout;
	}

	// the gidentity string is of the form: 'sector: 2, layer: 4, wire; 33'
	// by construction in the sci-g API
	vector<string> identity = gutilities::getStringVectorFromStringWithDelimiter(gidentityString, ",");
	// each identity item is a string of the form 'sector: 2'
	for ( auto& gid: identity) {
		vector<string> identifier = gutilities::getStringVectorFromStringWithDelimiter(gid, ":");

		string idName  = identifier[0];
		int idValue = stoi(identifier[1]);

		gidentity.push_back(GIdentifier(idName, idValue));
	}

}


// copy constructor called in the non-overloaded processTouchable:
// used in case the stepTimeIndex of the hit is different from the gtouchable one
GTouchable::GTouchable(const GTouchable* baseGT, int newTimeIndex) {
    gType         = baseGT->gType;
    gidentity     = baseGT->gidentity;
    verbosity     = baseGT->verbosity;
    trackId       = baseGT->trackId;
    eMultiplier   = baseGT->eMultiplier;
    detectorDimensions = baseGT->detectorDimensions;
    stepTimeAtElectronicsIndex = newTimeIndex;
}



// copy constructor called in processTouchable
// weight and time can be set by processTouchable
//GTouchable::GTouchable(const GTouchable& baseGT, vector<GIdentifier> gid, float weight) :
//gidentity(gid),
//eMultiplier(weight) {
//	gType             = baseGT.gType;
//	verbosity         = baseGT.verbosity;
//	trackId           = baseGT.trackId;
//	stepTimeAtElectronics = baseGT.stepTimeAtElectronics;
//}

// copy constructor called in processTouchable
// weight and time can be set by processTouchable
//GTouchable::GTouchable(const GTouchable& baseGT, vector<GIdentifier> gid, float weight, float t) :
//gidentity(gid),
//eMultiplier(weight),
//stepTimeAtElectronics(t) {
//	gType         = baseGT.gType;
//	verbosity     = baseGT.verbosity;
//	trackId       = baseGT.trackId;
//}


// todo: optimize the algorithm
// Overloaded "==" operator for the class 'GTouchable'
bool GTouchable::operator == (const GTouchable& that) const
{
    if (verbosity) {
        cout << " Touchable comparison:  " << endl;
        for (size_t i=0; i<that.gidentity.size(); i++) {
            string comparisonResult = ( this->gidentity[i].getValue() == that.gidentity[i].getValue() ) ? " ✅" : " ❌";
            cout << "   " << this->gidentity[i] << " " <<  that.gidentity[i] << comparisonResult <<  endl;
        }
        if (this->gType == readout) {
            string cellIndexCompariton = (this->stepTimeAtElectronicsIndex == that.stepTimeAtElectronicsIndex) ? " ✅" : " ❌";
            cout << "   time index: " << this->stepTimeAtElectronicsIndex << " " << that.stepTimeAtElectronicsIndex << cellIndexCompariton << endl;
        }
    }

    // first, compare size of identity
	// this should never happen because the same sensitivity should be assigned the same identifier structure
	if (this->gidentity.size() != that.gidentity.size()) {
		if (verbosity) {
			cout << " Touchable sizes are different " << endl;
		}
		return false;
	}



	// now compare that the identity is actually the same
	// return false if something is different
	for (size_t i=0; i<that.gidentity.size(); i++) {
		if ( this->gidentity[i].getValue() != that.gidentity[i].getValue() ) {
            if (verbosity) {
                cout << " Touchable gidentity  are different: " << this->gidentity[i] << " " << that.gidentity[i] << endl;
            }
			return false;
		}
	}

	// all identities are the same
	// now using gtouchable type
	switch (this->gType) {
		case readout:
			return this->stepTimeAtElectronicsIndex == that.stepTimeAtElectronicsIndex ;
		case flux:
			return this->trackId == that.trackId;
		case dosimeter:
			return this->trackId == that.trackId;
		case particleCounter:
			return true;
	}

	return false;
}

// ostream GTouchable
ostream &operator<<(ostream &stream, GTouchable gtouchable) {

	stream << " GTouchable: ";
	for ( auto& gid: gtouchable.gidentity ) {
		stream << KRED << gid ;
		if ( gid.getName() != gtouchable.gidentity.back().getName() ) {
			stream << ", ";
		} else {
			stream << RST ;
		}
	}
	switch (gtouchable.gType) {
		case readout:
			// compare the time cell
			stream << KGRN << " (readout), " << RST << " multiplier: " << gtouchable.eMultiplier << ", time cell index: " << gtouchable.stepTimeAtElectronicsIndex ;
			break;
		case flux:
			stream << KGRN << " (flux), " << RST << " g4 track id: " << gtouchable.trackId;
			break;
		case dosimeter:
			stream << KGRN << " (dosimeter), " << RST << " g4 track id: " << gtouchable.trackId ;
			break;
		case particleCounter:
			stream << KGRN << " (particleCounter), " << RST << " g4 track id: " << gtouchable.trackId ;
			break;
	}


	return stream;
}

// ostream GIdentifier
ostream &operator<<(ostream &stream, GIdentifier gidentifier) {
	stream << gidentifier.idName << ": " <<  gidentifier.idValue ;
	return stream;
}
