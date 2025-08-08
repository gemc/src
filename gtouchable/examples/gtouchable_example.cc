// gtouchable
#include "gtouchable.h"
#include "gtouchable_options.h"

// gemc
#include "glogger.h"

using std::string;
using std::vector;

int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, gtouchable::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, TOUCHABLE_LOGGER); // duplicate a touchable logger

	auto a_ctof_gtouchable = GTouchable(gopts, "readout", "sector: 5, paddle: 5", {10.0, 20.0, 30.0});

	for (unsigned i = 1; i < 10; i++) {
		GTouchable ctof = *GTouchable::create(log);

		bool is_equal = ctof == a_ctof_gtouchable;

		log->info(" GTouchable: ", ctof, " is equal: ", is_equal ? "true" : "false");
	}


	return EXIT_SUCCESS;
}
