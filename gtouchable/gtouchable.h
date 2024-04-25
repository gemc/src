#ifndef  GTOUCHABLE_H
#define  GTOUCHABLE_H 1

// c++
#include <vector>
#include <string>
#include <iostream>

using std::string;
using std::ostream;
using std::vector;

// - readout: electronic Time Window is the discriminating factor.
//   parameters and hitBitSet determined by defineReadoutSpecs in the plugin
// - flux: track id is the discriminating factor, standard true infos variable
// - particleCounter: no other discriminating factors, standard true infos variable
// - dosimeter: track id is the discriminating factor, radiation digitization
enum GTouchableType {
    readout, flux, particleCounter, dosimeter
};

struct GIdentifier {

public:

    // constructor
    GIdentifier(string n, int v) : idName{n}, idValue{v} {}

    //  Overloaded "=="
    bool operator==(const GIdentifier &gid) const { return this->idValue == gid.idValue; }

    // api
    inline string getName() const { return idName; }

    inline int getValue() const { return idValue; }

private:
    string idName;
    int idValue;

    // Logs GIdentifier on screen
    friend ostream &operator<<(ostream &stream, GIdentifier gidentifier);
};

// GTouchable:
// 1. Always use gidentifier as the main discriminating factor.
// 2. If the gidentifier are the same, gType is used
// Criteria in the overloaded == 
class GTouchable {

public:
    // constructor called in GDetectorConstruction::ConstructSDandField
    // to register a new gtouchable in the sensitive detector gtouchable map
    GTouchable(string digitization, string gidentityString, vector<double> dimensions, bool verb = false);

    // copy constructor called in the non-overloaded processTouchable:
    // used in case the stepTimeIndex of the hit is different from the gtouchable one
    GTouchable(const GTouchable *baseGT, int newTimeIndex);


    // copy constructor called in processTouchable
    // used for energy sharing (keeps time as is)
    // need to provide the gidentity
    //GTouchable(const GTouchable& baseGT, vector<GIdentifier> gidentity, float weight);

    // copy constructor called in processTouchable
    // used for energy sharing and time propagation
    // need to provide the gidentity
    //GTouchable(const GTouchable& baseGT, vector<GIdentifier> gidentity, float weight, float t);

private:
    // set by sensitive detector constructor
    GTouchableType gType;
    vector <GIdentifier> gidentity;  // Uniquely identify a sensitive element
    bool verbosity;

    // set in sensitiveDetector::ProcessHit
    // Used to determine if steps belong to the same hit for flux/particle counter detectors
    int trackId;

    // set by processGTouchable in the digitization plugin. Defaulted to 1. Used to share energy / create new hits.
    // Energy Multiplier. By default, it is 1, but energy could be shared (or created) among volumes
    float eMultiplier;

    // stepTimeAtElectronicsIndex is used to determine if a hit is within
    // an existing detector readout electronic time window
    // stepTimeAtElectronicsIndex is set using assignStepTimeAtElectronicsIndex
    // in gDynamicDigitization using the greadoutSpecs
    int stepTimeAtElectronicsIndex;

    // to print it out
    friend ostream &operator<<(ostream &stream, GTouchable gtouchable);

    // could be used in GDynamicDigitization when the dimensions are needed
    vector<double> detectorDimensions;

public:
    // Overloaded "==" operator for the class 'GTouchable'
    bool operator==(const GTouchable &gtouchable) const;

    // called in GSensitiveDetector::ProcessHits
    // void assignTimeAtElectronics(float t) {timeAtElectronics = t;}
    inline void assignTrackId(int tid) { trackId = tid; }

    inline float getEnergyMultiplier() const { return eMultiplier; }

    inline void assignStepTimeAtElectronicsIndex(int timeIndex) { stepTimeAtElectronicsIndex = timeIndex; }

    inline int getStepTimeAtElectronicsIndex() const { return stepTimeAtElectronicsIndex; }

// api
public:
    inline const vector <GIdentifier> getIdentity() const { return gidentity; }

    inline const vector<double> getDetectorDimensions() const { return detectorDimensions; }

};


#endif
