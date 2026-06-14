/**
* \file gdynamicdigitization_options.cc
 * \brief Implementation of option aggregation for gdynamic digitization.
 *
 * This file defines gdynamicdigitization::defineOptions(), which creates the option set
 * for this module and aggregates options from dependent subsystems.
 */

#include "gdynamicdigitization_options.h"
#include "gtranslationTable_options.h"
#include "gfactory_options.h"
#include "gEventDataCollection.h"
#include "gRunDataCollection.h"

namespace gdynamicdigitization {

GOptions defineOptions() {
	// Create an option container scoped to the digitization logger name.
	GOptions goptions(GDIGITIZATION_LOGGER);

	// When enabled, hits with exactly zero deposited energy are still recorded.
	goptions.defineSwitch("recordZeroEdep", "Record particle even if they do not deposit energy in the sensitive volumes");

	// When set, overrides the per-system geometry variation used to load digitization
	// constants and translation tables. The sentinel default means "not set": each routine
	// then follows the variation of the gsystem it belongs to.
	goptions.defineOption(
		GVariable("digitization_variation", UNINITIALIZEDSTRINGQUANTITY, "digitization variation"),
		"If set, overrides the gsystem variation when the digitization routines load their\n"
		"constants and translation tables. Default: not set (each routine uses its system's\n"
		"variation).\n \nExample: -digitization_variation=rga_fall2018");

	// Aggregate options required by downstream types used in the digitization workflow.
	goptions += gevent_data::defineOptions();
	goptions += grun_data::defineOptions();
	goptions += gtranslationTable::defineOptions();
	goptions += gfactory::defineOptions();

	return goptions;
}

} // namespace gdynamicdigitization
