// gtouchable
#include "gtouchable.h"
#include "gtouchable_options.h"

// gemc
#include "glogger.h"

using std::string;
using std::vector;

int main(int argc, char* argv[]) {
	auto gopts = new GOptions(argc, argv, gtouchable::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, TOUCHABLE_LOGGER, "gtouchable example");


	auto a_ctof_gtouchable = GTouchable("readout",  "sector: 6, paddle: 10", {10.0, 20.0, 30.0}, log);

	for (unsigned i = 1; i < 100; i++) {
		GTouchable ctof = *GTouchable::create(log);

		bool is_equal = ctof == a_ctof_gtouchable;

		if (i % 10 == 0) { log->info(" GTouchable: ", ctof, " is equal: ", is_equal? "true" : "false"); }
	}


	// clean up,
	// deleting log here gives error on linux. should be investigated
	delete gopts;

	return EXIT_SUCCESS;
}
