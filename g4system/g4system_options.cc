// g4system
#include "g4system_options.h"
#include "g4systemConventions.h"
#include "gsystem_options.h"

// project goption to a system
namespace g4system {

// returns array of options definitions
GOptions defineOptions() {
	GOptions goptions(G4SYSTEM_LOGGER);
	goptions += gsystem::defineOptions();

	string help;

	// help = "All information related to the volume will be logged. \n \n";
	// help += "Example: -logVolume=ctof\n";
	// goptions.defineOption(GVariable("logVolume", goptions::NODFLT, "log all information for volume"), help);


	help =
		"Material to be used if some volume requested a non-defined material.  \n \n";
	help += "By default gemc will exit with error.\n";
	help += "Example: -useBackupMaterial=G4_Air\n";
	goptions.defineOption(GVariable("useBackupMaterial", NO_USE_DEFAULT_MATERIAL,
	                                "Backup material"), help);

	// TODO: check overlaps
	// help = "Check for volumes overlaps.\n \n";
	// help += "Possibles values are:\n";
	// help += " - 0 (default): no check.\n";
	// help += " - 1: check for overlaps at physical volume construction.\n";
	// help += " - 2: use the geant4 overlap validator with 10,000 points on the surface\n";
	// help += " - Any N greater than 100 : use the geant4 overlap validator with N points on the surface";
	// goptions.defineOption(GVariable("checkOverlapsForVolume", 0, "check for volumes overlaps"), help);

	goptions.defineSwitch("showPredefinedMaterials", "log GEMC Predefined Materials");
	goptions.defineSwitch("printSystemsMaterials", "print the materials used in this simulation");
	goptions.defineSwitch("checkOverlaps", "check geant4 volume overlaps at construction time");

	// check overlaps
	help = "Check for overlaps at physical volume construction.\n \n";
	help += "Possibles values are:\n";
	help += " - 0 (default): no check.\n";
	help += " - 1: check for overlaps at physical volume construction.\n";
	help += " - 2: use the geant4 overlap validator with 10,000 points on the surface\n";
	help += " - Any N greater than 100 : use the geant4 overlap validator with N points on the surface";
	goptions.defineOption(GVariable("check_overlaps", 0, "check overlaps"), help);

	return goptions;
}


}
