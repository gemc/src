#include "gdata_options.h"
#include "gtouchable_options.h"

// project goption to a system
namespace gdata {

// returns array of options definitions
GOptions defineOptions() {

	GOptions goptions("gdata");
	goptions += gtouchable::defineOptions();

	return goptions;
}

}
