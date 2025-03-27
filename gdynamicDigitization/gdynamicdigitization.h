#ifndef  GDYNAMICDIGITIZATION_H
#define  GDYNAMICDIGITIZATION_H 1

// dynamic digitization
#include "greadoutSpecs.h"
#include "gfactory_options.h"

// gemc
#include "gdl.h"
#include "gtouchable.h"
#include "ghit.h"
#include "gDigitizedData.h"
#include "gTrueInfoData.h"
#include "gtranslationTable.h"
#include "gfactory_options.h"
#include "gtranslationTable_options.h"
#include "gdata_options.h"
#include "gdynamicdigitization_options.h"


// c++
#include <vector>
#include <bitset>

// geant4
#include "G4Step.hh"

class GTouchableModifiers {

public:
    explicit GTouchableModifiers(vector <string> touchableNames);

private:
    // only one of these maps can be filled with values:
    // the size of the map is used by processGTouchableModifiers

    // vector is pair: (id, weight)
    map <string, vector<double>> modifierWeightsMap;

    // vector is triplet: (id, weight, time)
    map <string, vector<double>> modifierWeightsAndTimesMap;

// api
public:

    // insert new values
    void insertIdAndWeight(string touchableName, int idValue, double weight);

    void insertIdWeightAndTime(string touchableName, int idValue, double weight, double time);

    // normalize a map using totalWeight
    void assignOverallWeight(string touchableName, double totalWeight);

    inline bool isWeightsOnly() {
        return !modifierWeightsMap.empty();
    }

    // get vectors from modifierWeightsMap using touchableName
    inline vector<double> getModifierWeightsVector(const string& touchableName) {
        // this will crash if user request a key not declared in the constructor
        return modifierWeightsMap[touchableName];
    }

    // get vectors from modifierWeightsAndTimesMap using touchableName
    inline vector<double> getModifierWeightsAndTimeVector(const string& touchableName) {
        // this will crash if user request a key not declared in the constructor
        return modifierWeightsAndTimesMap[touchableName];
    }
};


class GDynamicDigitization {

public:
    // not declared: default constructor

    // abstract destructor
    virtual ~GDynamicDigitization() = default;

    // the time used in processTouchable
    virtual float processStepTime(GTouchable *gTouchID, G4Step *thisStep);

    // change the GTouchable in sensitiveDetector::ProcessHit
    // by default the touchable is not changed and the gtouchable is assigned a time index based
    // on the readout specs and the step time
    // this function can be overloaded by plugin methods
    // notice that this returns a vector of touchables, as one g4step can produce multiple hits
    virtual vector<GTouchable *> processTouchable(GTouchable *gTouchID, G4Step *thisStep);

    // need to document exactly what this does and if it's still needed
    vector<GTouchable *> processGTouchableModifiers(GTouchable *gTouchID, GTouchableModifiers gmods);

    // filter true information into GTrueInfoHit
    // this integrates all available information built in GHit::addHitInfosForBitset
    GTrueInfoData *collectTrueInformation(GHit *ghit, size_t hitn);

    // digitize true information into GDigitizedHit. Suppressing warning for unused parameter
    virtual GDigitizedData *digitizeHit([[maybe_unused]] GHit *ghit, [[maybe_unused]] size_t hitn) { return nullptr; }

    // loads the digitization constants
    // return false for failure
    virtual bool loadConstants([[maybe_unused]] int runno, [[maybe_unused]] string variation) { return false; }

    // loads the translation table
    // return false for failure
    virtual bool loadTT([[maybe_unused]] int runno, [[maybe_unused]] string variation) { return false; }

    // this will set the gdata variable TIMEATELECTRONICS used by RunAction to identify the eventFrameIndex
    // and will include in gdata the translation table (hardware address crate/slot/channel)
    // this will exit with error if the TT is not defined
    // notice time is an int (assumed unit: ns)
    void chargeAndTimeAtHardware(int time, int q, GHit *ghit, GDigitizedData *gdata);

    // mandatory initialization of readout specs
    virtual bool defineReadoutSpecs() = 0;

    GReadoutSpecs *readoutSpecs = nullptr;
    GTranslationTable *translationTable = nullptr;

    // method to dynamically load factories
    static GDynamicDigitization *instantiate(const dlhandle handle) {

        if (handle == nullptr) return nullptr;

        // must match the extern C declaration in the derived factories
        void *maker = dlsym(handle, "GDynamicDigitizationFactory");

        if (maker == nullptr) return nullptr;

        typedef GDynamicDigitization *(*fptr)();

        // static_cast not allowed here
        // see http://stackoverflow.com/questions/573294/when-to-use-reinterpret-cast
        // need to run the DLL GDynamicFactory function that returns the factory
        fptr func = reinterpret_cast<fptr>(reinterpret_cast<void *>(maker));

        return func();
    }

	void set_loggers(GOptions *const g) {
		gopts = g;
		data_logger = new GLogger(gopts.value(), DATA_LOGGER);
		tt_logger = new GLogger(gopts.value(), TRANSLATIONTABLE_LOGGER);
		digi_logger = new GLogger(gopts.value(), GDIGITIZATION_LOGGER);
	}

protected:

	std::optional<GOptions *> gopts;
	std::optional<GLogger *> data_logger;
	std::optional<GLogger *> tt_logger;
	std::optional<GLogger *> digi_logger;

	// should be able to remove this once we verify that the logger is set
	void check_if_log_defined() {
		if ( !gopts.has_value() || !data_logger.has_value() || !tt_logger.has_value() || !digi_logger.has_value() ) {
			std::cerr << KRED << "Fatal Error: GDynamicDigitization: goption is not set for this plugin." << std::endl;
			std::cerr << "The set_loggers function need to be called. " << std::endl;
			std::cerr << "For example: dynamicRoutines[\"ctof\"]->set_loggers(gopts); " << RST << std::endl;
			exit(1);
		}
	}


};

#endif
