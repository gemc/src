#include "g4dialog_options.h"

#include "g4display_options.h"

// namespace to define options
namespace g4dialog {
GOptions defineOptions() {
	// Summary: Define and return the option schema for the g4dialog module.

	GOptions goptions(G4DIALOG_LOGGER);

	return goptions;
}
} // namespace g4dialog
