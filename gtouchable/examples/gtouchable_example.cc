// gtouchable example
//
// Demonstrates basic usage of the gtouchable module:
// - Create a module option set and logger.
// - Build a reference GTouchable from a digitization type and identity string.
// - Create additional test touchables via GTouchable::create() and compare them.

#include "gtouchable.h"
#include "gtouchable_options.h"

// gemc
#include "glogger.h"

using std::string;
using std::vector;

int main(int argc, char* argv[]) {
	// Build module options and create a logger instance dedicated to this example.
	// The logger is configured with the gtouchable module logger name (TOUCHABLE_LOGGER).
	auto gopts = std::make_shared<GOptions>(argc, argv, gtouchable::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, TOUCHABLE_LOGGER); // duplicate a touchable logger

	// Reference touchable used for comparisons.
	auto a_ctof_gtouchable = GTouchable(gopts, "readout", "sector: 5, paddle: 5", {10.0, 20.0, 30.0});

	for (unsigned i = 1; i < 10; i++) {
		// Create a synthetic test touchable with a deterministic identity pattern.
		GTouchable ctof = *GTouchable::create(log);

		// Compare using GTouchable::operator== semantics:
		// - identity values first
		// - then a type-specific discriminator
		bool is_equal = ctof == a_ctof_gtouchable;

		log->info(" GTouchable: ", ctof, " is equal: ", is_equal ? "true" : "false");
	}

	return EXIT_SUCCESS;
}
