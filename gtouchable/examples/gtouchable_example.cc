// gtouchable
#include "gtouchable.h"
#include "gtouchable_options.h"

// gemc
#include "glogger.h"

int main(int argc, char* argv[]) {
	auto gopts = new GOptions(argc, argv, gtouchable::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, TOUCHABLE_LOGGER, "gtouchable example");

	string         first_identity   = "sector: 98, paddle: 98";
	vector<double> first_dimensions = {1.0, 20.0, 98.0};
	auto           first_ctof       = GTouchable("readout", first_identity, first_dimensions, log);

	for (unsigned i = 1; i < 100; i++) {
		string         identity   = "sector: " + std::to_string(i) + ", paddle: " + std::to_string(i);
		vector<double> dimensions = {1.0, 20.0, i * 1.0};
		GTouchable     ctof("readout", identity, dimensions, log);

		bool is_equal = ctof == first_ctof;

		if (i % 10 == 0) { log->info(" GTouchable: ", ctof, " is equal: ", is_equal); }
	}

	// clean up,
	// deleting log here gives error on linux. should be investigated
	delete gopts;

	return EXIT_SUCCESS;
}
