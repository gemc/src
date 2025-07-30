// gemc
#include "glogger.h"
#include "gemc_options.h"
#include "gemcConventions.h"

// options definitions
#include "dbselect_options.h"
#include "gstreamer_options.h"
#include "gsplash.h"
#include "gphysics_options.h"
#include "gaction.h"
#include "gparticle_options.h"
#include "eventDispenser_options.h"
#include "g4display_options.h"
#include "g4dialog_options.h"

namespace gemc {

    // returns array of options definitions
    GOptions defineOptions() {
        GOptions goptions(GENERAL_LOGGER);

        // switches
        goptions.defineOption(GVariable("nthreads", 0, "sets number of threads."), "Default: 0 (use one thread for each available cores)");
        // goptions.defineOption(GVariable("event_module_log", 0, "Event Modulo log"), "Logs every <value> events. Default: 0 (log all events)");
        //
        // goptions.defineOption(GVariable("digitization_variation", "default", "digitization variation"), "Default: default");
        // goptions.defineOption(GVariable("eventTimeSize", "0*ns", "event duration with unit"), "Default: 0*ns");

        // random engine name
        // the names, that come from the CLHEP library, can be found with
        // grep ": public HepRandomEngine" *.h $CLHEP_BASE_DIR/include/CLHEP/Random/* | awk -Fclass '{print $2}' | awk -F: '{print $1}'
        std::string help = "Random Engine Name: set the CLHEP:HepRandomEngine. Default: " + std::string(DEFAULT_RANDOM_ENGINE) + "\n";
        help += std::string(HELPFILLSPACE) + "The available CLHEP:HepRandomEngine are:\n\n";
        help += std::string(HELPFILLSPACE) + " - DRand48Engine\n";
        help += std::string(HELPFILLSPACE) + " - DualRand\n";
        help += std::string(HELPFILLSPACE) + " - Hurd160Engine\n";
        help += std::string(HELPFILLSPACE) + " - Hurd288Engine\n";
        help += std::string(HELPFILLSPACE) + " - HepJamesRandom\n";
        help += std::string(HELPFILLSPACE) + " - MTwistEngine\n";
        help += std::string(HELPFILLSPACE) + " - NonRandomEngine\n";
        help += std::string(HELPFILLSPACE) + " - RandEngine\n";
        help += std::string(HELPFILLSPACE) + " - RanecuEngine\n";


        help += std::string(HELPFILLSPACE) + " - Ranlux64Engine\n";
        help += std::string(HELPFILLSPACE) + " - RanluxEngine\n";
        help += std::string(HELPFILLSPACE) + " - RanluxppEngine final\n";
        help += std::string(HELPFILLSPACE) + " - RanshiEngine\n";
        help += std::string(HELPFILLSPACE) + " - TripleRand\n";
        goptions.defineOption(GVariable("randomEngine", DEFAULT_RANDOM_ENGINE, "randomEngine"), help);

        // random seed
        help = "Random Seed: set the random seed to an integer value. Default: \n";
        help += std::string(HELPFILLSPACE) + "If the random seed is set to -12345, the seed will be set using a combinration of: \n";
        help += std::string(HELPFILLSPACE) + " - local time \n";
        help += std::string(HELPFILLSPACE) + " - process id \n";
        help += std::string(HELPFILLSPACE) + " - clock function \n";
        goptions.defineOption(GVariable("seed", SEEDNOTSET, "randomEngineName"), help);


		// load other
		goptions += dbselect::defineOptions(); // includes gdetector (gdynamicdigitization (gdata (gtouchable) , gtranslationTable, gfactory), g4system, gsystem)
    	goptions += gstreamer::defineOptions();
    	goptions += gsplash::defineOptions();
    	goptions += gphysics::defineOptions();
    	goptions += gaction::defineOptions();
    	goptions += gparticle::defineOptions();
    	goptions += eventDispenser::defineOptions();
    	goptions += g4display::defineOptions();
    	goptions += g4dialog::defineOptions();


        return goptions;
    }


}
