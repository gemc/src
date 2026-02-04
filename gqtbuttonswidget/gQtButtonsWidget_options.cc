#include "gQtButtonsWidget_options.h"

// Namespace containing option definitions for the Qt Buttons Widgets module.
namespace gqtbuttonswidget {
GOptions defineOptions() {
	// Construct an options container using this module's logger name.
	// Applications may add switches/scalars to this object as the module evolves.
	GOptions goptions(GQTBUTTONS_LOGGER);

	// Currently no module-specific options are registered here.
	// The function returns the container for consistency across modules.
	return goptions;
}
}
