// gemc
#include "gutsConventions.h"
#include "goptions.h"
#include "gemcOptions.h"

// options definitions
#include "eventDispenserOptions.h"
#include "g4displayOptions.h"
#include "g4systemOptions.h"
#include "gfieldOptions.h"
#include "gparticleOptions.h"
#include "gphysicsOptions.h"
#include "gstreamerOptions.h"
#include "gsystemOptions.h"

// c++
#include <iostream>
using namespace std;


namespace gemc {

	// returns array of options definitions
	vector<GOption> defineOptions()
	{
		vector<GOption> goptions;

		// "stream" switch
		goptions.push_back(GOption("stream", "Activate Streaming ReadOut"));
		
		// record particle even if they do not deposit energy in the sensitive volumes
		goptions.push_back(GOption("recordZeroEdep", "Record particle even if they do not deposit energy in the sensitive volumes"));

		// "print option" switch
		goptions.push_back(GOption("sndf", "Shows non default options"));

        // show on screen the G4 threads log
        goptions.push_back(GOption("showG4ThreadsLog", "show on screen the G4 threads log"));

        // number of threads. Default = 0 (all)
		json jsonNThreadOption = {
			{GNAME, "nthreads"},
			{GDESC, "set number of threads"},
			{GDFLT, 0}
		};
		goptions.push_back(GOption(jsonNThreadOption));

		// general verbosity
		json jsonMessageVerbosity = {
			{GNAME, "verbosity"},
			{GDESC, GVERBOSITY_DESCRIPTION},
			{GDFLT, 0}
		};
		goptions.push_back(GOption(jsonMessageVerbosity));

		// run verbosity
		json jsonRunVerbosity = {
			{GNAME, "grunv"},
			{GDESC, GVERBOSITY_DESCRIPTION},
			{GDFLT, 0}
		};
		goptions.push_back(GOption(jsonRunVerbosity));

		// sensitivity verbosity
		json jsonSensitivityVerbosity = {
			{GNAME, "gsensitivityv"},
			{GDESC, GVERBOSITY_DESCRIPTION},
			{GDFLT, 0}
		};
		goptions.push_back(GOption(jsonSensitivityVerbosity));

		// event log every N events
		json jsonELOGOption = {
			{GNAME, "elog"},
			{GDESC, "Log every N events"},
			{GDFLT, 1000}
		};
		goptions.push_back(GOption(jsonELOGOption));

		// plugins search path
		json jsonPluginPathOption = {
			{GNAME, "gpluginsPath"},
			{GDESC, "Directory containing the plugins"},
			{GDFLT, UNINITIALIZEDSTRINGQUANTITY}
		};
		goptions.push_back(GOption(jsonPluginPathOption));

		// log only thread # given
		json jsonTLOGOption = {
			{GNAME, "tlog"},
			{GDESC, "Log only thread # given. 0 (default) means log all"},
			{GDFLT, 0}
		};
		goptions.push_back(GOption(jsonTLOGOption));

		// digitization variation
		json jsonDigiVariationOption = {
			{GNAME, "dVariation"},
			{GDESC, "Digitization Variation"},
			{GDFLT, "default"}
		};
		goptions.push_back(GOption(jsonDigiVariationOption));


		// event duration
		json jsonEventDurationOption = {
			{GNAME, "eventTimeSize"},
			{GDESC, "event duration with unit. Examples: 4*ns, 2*ms"},
			{GDFLT, "0*ns"}
		};
		goptions.push_back(GOption(jsonEventDurationOption));


        // random engine name
        // the names come from the CLHEP library, can be found with
        // grep ": public HepRandomEngine" *.h $CLHEP_BASE_DIR/include/CLHEP/Random/* | awk -Fclass '{print $2}' | awk -F: '{print $1}'
        string randomHelp = "Random Engine Name: set the CLHEP:HepRandomEngine. Default: MixMaxRng\n";
        randomHelp += string(HELPFILLSPACE) + "The available CLHEP:HepRandomEngine are:\n\n";
        randomHelp += string(HELPFILLSPACE) + " - DRand48Engine\n";
        randomHelp += string(HELPFILLSPACE) + " - DualRand\n";
        randomHelp += string(HELPFILLSPACE) + " - Hurd160Engine\n";
        randomHelp += string(HELPFILLSPACE) + " - Hurd288Engine\n";
        randomHelp += string(HELPFILLSPACE) + " - HepJamesRandom\n";
        randomHelp += string(HELPFILLSPACE) + " - MTwistEngine\n";
        randomHelp += string(HELPFILLSPACE) + " - MixMaxRng\n";
        randomHelp += string(HELPFILLSPACE) + " - RandEngine\n";
        randomHelp += string(HELPFILLSPACE) + " - RanecuEngine\n";
        randomHelp += string(HELPFILLSPACE) + " - Ranlux64Engine\n";
        randomHelp += string(HELPFILLSPACE) + " - RanluxEngine\n";
        randomHelp += string(HELPFILLSPACE) + " - RanluxppEngine\n";
        randomHelp += string(HELPFILLSPACE) + " - RanshiEngine\n";
        randomHelp += string(HELPFILLSPACE) + " - TripleRand\n";
        json jsonRandomEngineNameOption = {
            {GNAME, "randomEngineName"},
            {GDESC, randomHelp},
            {GDFLT, "MixMaxRng"}
        };
        goptions.push_back(GOption(jsonRandomEngineNameOption));

        // random seed
        string randomSeedHelp = "Random Seed: set the random seed to an integer value. Default: -99\n";
        randomSeedHelp += string(HELPFILLSPACE) + "If the random seed is set to -99, the seed will be set using: \n";
        randomSeedHelp += string(HELPFILLSPACE) + " - local time \n";
        randomSeedHelp += string(HELPFILLSPACE) + " - process id \n";
        randomSeedHelp += string(HELPFILLSPACE) + " - clock function \n";
        json jsonRandomSeedOption = {
            {GNAME, "randomSeed"},
            {GDESC, randomSeedHelp},
            {GDFLT, -99}
        };
        goptions.push_back(GOption(jsonRandomSeedOption));


		// glibrary modules goptions
        goptions += eventDispenser::defineOptions();
        goptions += g4display::defineOptions();
        goptions += g4system::defineOptions();
        goptions += gfield::defineOptions();
        goptions += gparticle::defineOptions();
        goptions += gphysics::defineOptions();
        goptions += gstreamer::defineOptions();
		goptions += gsystem::defineOptions();

		return goptions;
	}


}
