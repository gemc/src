/**
 * \file gdynamicdigitization_options.cc
 * \brief Implementation of GDynamicDigitization options definitions.
 *
 * This file implements the function that creates and aggregates the options for
 * the GDynamicDigitization module.
 */

#include "gdynamicdigitization_options.h"
#include "gtranslationTable_options.h"
#include "gfactory_options.h"
#include "gEventDataCollection.h"
#include "gRunDataCollection.h"

namespace gdynamicdigitization {

GOptions defineOptions() {

	GOptions goptions(GDIGITIZATION_LOGGER);

	goptions.defineSwitch("recordZeroEdep", "Record particle even if they do not deposit energy in the sensitive volumes");

	goptions += gevent_data::defineOptions();
	goptions += grun_data::defineOptions();
	goptions += gtranslationTable::defineOptions();
	goptions += gfactory::defineOptions();

	return goptions;

}

} // namespace gdata
