// gemc
#include "glogger.h"
#include "gemc_options.h"
#include "gemcConventions.h"

// yaml-cpp — needed by collectPluginOptions for the bootstrap YAML scan
#include "yaml-cpp/yaml.h"

// POSIX dynamic loading
#include <dlfcn.h>

// c++
#include <filesystem>
#include <set>
#include <sstream>

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
#include "gboard.h"
#include "gsd.h"
#include "gfield_options.h"
#include "gtree_options.h"
#include "pmaker_options.h"

namespace gemc {

    // returns array of options definitions
    GOptions defineOptions() {
        GOptions goptions(GENERAL_LOGGER);

        // switches
        goptions.defineOption(GVariable("nthreads", 0, "sets number of threads."), "Default: 0 (use one thread for each available cores)");
        goptions.defineOption(
            GVariable("also_reject_true_info", true, "also reject true information for rejected hits"),
            "Drops true information for hits rejected by detector digitization. Default: true.\n"
            "Set to false to keep true information for all Geant4 hits even when no digitized hit is written.");
        // goptions.defineOption(GVariable("event_module_log", 0, "Event Modulo log"), "Logs every <value> events. Default: 0 (log all events)");
        //
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
        help += std::string(HELPFILLSPACE) + "If the random seed is set to -12345, the seed will be set using a combination of: \n";
        help += std::string(HELPFILLSPACE) + " - local time \n";
        help += std::string(HELPFILLSPACE) + " - process id \n";
        help += std::string(HELPFILLSPACE) + " - clock function \n";
        goptions.defineOption(GVariable("seed", SEEDNOTSET, "seed"), help);


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
    	goptions += gsensitivedetector::defineOptions();
    	goptions += gfields::defineOptions();
    	goptions += gboard::defineOptions();
    	goptions += gtree::defineOptions();
    	goptions += pmaker::defineOptions();


        return goptions;
    }


    GOptions collectPluginOptions(int argc, char* argv[]) {
        GOptions merged;

        // Build the plugin search path from three sources, checked in priority order:
        //   1. -plugin_path=... on the command line (not yet parsed)
        //   2. plugin_path: value in any YAML file listed in argv
        //   3. GEMC_PLUGIN_PATH environment variable
        // Source 1 is extracted first; source 2 is accumulated while scanning YAML files
        // below; source 3 is appended after both.
        std::string search_path;
        for (int i = 1; i < argc; ++i) {
            std::string_view arg = argv[i];
            if (arg.size() > 13 && arg.substr(0, 13) == "-plugin_path=") {
                const auto val = std::string(arg.substr(13));
                if (!search_path.empty()) search_path += ':';
                search_path += val;
            }
        }

        // Scan every YAML file present in argv. For each file:
        //   - collect any plugin_path value declared there
        //   - collect gsystem and gstreamer plugin names to probe for a definePluginOptions symbol
        std::set<std::string> seen_names;  // deduplicate across multiple YAML files

        // Helper: resolve a plugin basename through the search path, open it, call
        // definePluginOptions() if the symbol exists, and merge the result into `merged`.
        auto probe_plugin = [&](const std::string& basename) {
            std::string full_search = search_path;
            if (const char* env = std::getenv("GEMC_PLUGIN_PATH")) {
                if (!full_search.empty()) full_search += ':';
                full_search += env;
            }

            std::string lib_path;
            if (!full_search.empty()) {
                std::istringstream ss(full_search);
                std::string        dir;
                while (std::getline(ss, dir, ':')) {
                    if (dir.empty()) continue;
                    const auto candidate = dir + "/" + basename;
                    if (std::filesystem::exists(candidate)) { lib_path = candidate; break; }
                }
            }
            if (lib_path.empty() && std::filesystem::exists(basename)) lib_path = basename;
            if (lib_path.empty()) return;  // no plugin — normal

            // On Linux, RTLD_NODELETE keeps it resident so that the later GManager dlopen
            // gets the exact same handle without re-executing static initialisers.
#if defined(__linux__) && defined(RTLD_NODELETE)
            const auto h = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_NODELETE);
#else
            const auto h = dlopen(lib_path.c_str(), RTLD_NOW);
#endif
            if (!h) return;  // library not loadable — skip silently

            // The definePluginOptions symbol is optional. Plugins that need no custom
            // options simply omit it and are silently skipped here.
            using opt_fptr = GOptions* (*)();
            const auto sym = reinterpret_cast<opt_fptr>(dlsym(h, "definePluginOptions"));
            if (!sym) return;

            std::unique_ptr<GOptions> plugin_opts(sym());
            merged += *plugin_opts;
        };

        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (!std::filesystem::exists(arg)) continue;

            YAML::Node root;
            try { root = YAML::LoadFile(arg); } catch (...) { continue; }

            // Pick up plugin_path declared in this YAML file (prepend so YAML paths
            // take priority over the env var but not over command-line paths).
            if (root["plugin_path"]) {
                try {
                    const auto yaml_pp = root["plugin_path"].as<std::string>();
                    if (!yaml_pp.empty()) {
                        search_path = search_path.empty()
                                      ? yaml_pp
                                      : search_path + ':' + yaml_pp;
                    }
                } catch (...) {}
            }

            // Probe gsystem plugins for definePluginOptions.
            if (root["gsystem"]) {
                for (const auto& entry : root["gsystem"]) {
                    if (!entry["name"]) continue;
                    std::string name;
                    try { name = entry["name"].as<std::string>(); } catch (...) { continue; }
                    if (name.empty()) continue;
                    if (seen_names.insert(name).second) probe_plugin(name + ".gplugin");

                    // A gsystem may be digitized by a plugin whose name differs from the system
                    // name (e.g. the EC and PCAL systems share the "ecal" plugin). Probe that
                    // plugin too so its options and verbosity domain are registered before the
                    // factory is instantiated.
                    if (entry["digitization"]) {
                        std::string dig;
                        try { dig = entry["digitization"].as<std::string>(); } catch (...) { dig.clear(); }
                        if (!dig.empty() && seen_names.insert(dig).second) probe_plugin(dig + ".gplugin");
                    }
                }
            }

            // Probe gstreamer plugins for definePluginOptions so that verbosity domains
            // registered by those plugins (e.g. "hipo") are available before the factories
            // are instantiated.
            if (root["gstreamer"]) {
                for (const auto& entry : root["gstreamer"]) {
                    if (!entry["format"]) continue;
                    std::string format;
                    try { format = entry["format"].as<std::string>(); } catch (...) { continue; }
                    const std::string plugin_name = "gstreamer_" + format + "_plugin";
                    if (plugin_name.empty() || !seen_names.insert(plugin_name).second) continue;
                    probe_plugin(plugin_name + ".gplugin");
                }
            }
        }

        return merged;
    }

}
