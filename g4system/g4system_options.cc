// g4system_options.cc : option definitions for the g4system module.
/**
 * @file   g4system_options.cc
 * @ingroup g4system
 * @brief  Implementation of g4system option registration.
 *
 * @details
 * Header documentation in \c g4system_options.h is authoritative. This file contains the concrete
 * option and switch definitions assembled into a single GOptions instance.
 */

#include "g4system_options.h"
#include "g4systemConventions.h"
#include "gsystem_options.h"

// project goption to a system
namespace g4system {
// returns array of options definitions
GOptions defineOptions() {
	// Aggregate options from this module, factories, and the base gsystem module.
	GOptions goptions(G4SYSTEM_LOGGER);
	goptions += GOptions(G4SFACTORY_LOGGER);

	// Pull in shared system options (database, geometry system handling, etc.).
	goptions += gsystem::defineOptions();

	// Backup material option:
	// - used when a volume requests a material that is not defined in the Geant4 material tables
	// - default behavior is controlled by NO_USE_DEFAULT_MATERIAL
	std::string help =
		"Material to be used if some volume requested a non-defined material.\n\n";
	help += "By default GEMC will exit with error.\n";
	help += "Example: -useBackupMaterial=G4_Air\n";
	goptions.defineOption(GVariable("useBackupMaterial", NO_USE_DEFAULT_MATERIAL,
	                                "Backup material"), help);

	// Human-readable switches used for diagnostics and validation.
	goptions.defineSwitch("showPredefinedMaterials", "log GEMC Predefined Materials");
	goptions.defineSwitch("printSystemsMaterials", "print the materials used in this simulation");
	goptions.defineSwitch("checkOverlaps", "check geant4 volume overlaps at construction time");

	// Overlap checking mode:
	// - 0 disables overlap checks
	// - 1 enables overlap checking at placement time
	// - 2 uses Geant4 overlap validator with default surface sampling
	// - N>100 uses Geant4 overlap validator with N surface points
	help = "Check for overlaps at physical volume construction.\n\n";
	help += "Possible values are:\n";
	help += " - 0 (default): no check.\n";
	help += " - 1: check for overlaps at physical volume construction.\n";
	help += " - 2: use the geant4 overlap validator with the default number of points on the surface\n";
	help += " - Any N greater than 100: use the geant4 overlap validator with N points on the surface\n";
	goptions.defineOption(GVariable("check_overlaps", 0, "check overlaps"), help);

	return goptions;
}
}
