#include "gdetector_options.h"
#include "gsystem_options.h"
#include "g4system_options.h"
#include "gdynamicdigitization_options.h"
#include "gsd.h"

/**
 * @file gdetector_options.cc
 * @brief Implements the gdetector module option aggregation.
 *
 * Header documentation in gdetector_options.h remains authoritative for API usage.
 *
 * @ingroup gdetector_module
 */


// namespace to define options
namespace gdetector {

/**
 * @brief Builds the aggregated option set used by the gdetector module.
 *
 * This is a thin composition layer that merges option groups defined by the
 * dependent modules into a single returned object.
 *
 * @return Aggregated options instance tagged with the gdetector logger name.
 */
GOptions defineOptions() {
	// Create the root option container for this module.
	GOptions goptions(GDETECTOR_LOGGER);

	// Aggregate option groups required by the detector construction and digitization flow.
	goptions += gsystem::defineOptions();
	goptions += g4system::defineOptions();
	goptions += gdynamicdigitization::defineOptions();
	goptions += gsensitivedetector::defineOptions();

	return goptions;
}
}
