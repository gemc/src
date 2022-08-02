// goptions
#include "gutsConventions.h"
#include "goptions.h"

// options definitions
#include "gemcOptions.h"

// other modules goptions
#include "gsystemOptions.h"
#include "g4systemOptions.h"
#include "gstreamerOptions.h"
#include "g4displayOptions.h"
#include "eventDispenserOptions.h"
#include "gparticleOptions.h"

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

		// "print option" switch
		goptions.push_back(GOption("sndf", "Shows non default options"));

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


		// other modules goptions
		goptions += gsystem::defineOptions();
		goptions += g4system::defineOptions();
		goptions += gstreamer::defineOptions();
		goptions += g4display::defineOptions();
		goptions += eventDispenser::defineOptions();
		goptions += gparticle::defineOptions();

		return goptions;
	}


}



