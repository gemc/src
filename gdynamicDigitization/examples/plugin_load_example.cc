/// \file example.cc

// example on how to use the gdynamic library

// gdynamic
#include "gdynamicdigitization.h"
#include "gdynamicdigitization_options.h"

// gemc
#include "gfactory.h"
#include "gfactory_options.h"


int main(int argc, char *argv[]) {
	// Create GOptions using gdata::defineOptions, which aggregates options from gdata and gtouchable.
	auto gopts = new GOptions(argc, argv, gdynamicdigitization::defineOptions());

	// Create loggers: one for gdata and one for gtouchable.
	auto log = new GLogger(gopts, GDIGITIZATION_LOGGER, "plugin_load_example: main");
	auto plugin_log = std::make_shared<GLogger>(gopts, PLUGIN_LOGGER, "plugin_load_example: plugin manager");

    GManager manager(plugin_log, GDIGITIZATION_LOGGER);

    // using dynamicRoutines map
    std::map < std::string, GDynamicDigitization * > dynamicRoutines;
    dynamicRoutines["test"] = manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>("test_gdynamic_plugin", gopts);

    // using shared_ptr
    std::shared_ptr <GDynamicDigitization> globalCtof1(manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>("test_gdynamic_plugin", gopts));
	std::shared_ptr <GDynamicDigitization> globalCtof2(manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>("test_gdynamic_plugin", gopts));

    // increments reference count
    // to be used in the local thread
    auto globalCtof3(globalCtof2);

	log->info(0, "globalCtof1: ", globalCtof1, ", globalCtof2: ", globalCtof3, ", dynamicRoutines[\"ctof\"]: ", dynamicRoutines["test"]);

    dynamicRoutines["test"]->loadConstants(1, "default");
    globalCtof1->loadConstants(2, "default");
    globalCtof2->loadConstants(3, "target_shift");
    globalCtof3->loadConstants(4, "no_magnet");

	// clean up
	delete log;
	delete gopts;
    return EXIT_SUCCESS;
}
