/**
 * \file gdata_options.cpp
 * \brief Implementation of GData options definitions.
 *
 * This file implements the function that creates and aggregates the options for
 * the GData module.
 */

#include "gdata_options.h"
#include "gtouchable_options.h"

namespace gdata {

GOptions defineOptions() {
	GOptions goptions("gdata");
	goptions += gtouchable::defineOptions();
	return goptions;
}

} // namespace gdata
