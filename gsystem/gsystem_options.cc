// gsystem
#include "gsystem_options.h"
#include "gsystemConventions.h"

// project goption to a system
namespace gsystem {

// ADD EXPERIMENT, RUN NUMBER options read from general options


// method to return a vector of GSystem from the options
vector<GSystem> getSystems(GOptions* gopts, const std::shared_ptr<GLogger>& log) {
	vector<GSystem> systems;

	auto gsystem_node = gopts->getOptionNode("gsystem");

	for (auto gsystem_item : gsystem_node) {
		systems.emplace_back(
		                     log,
		                     gopts->get_variable_in_option<string>(gsystem_item, "name", goptions::NODFLT),
		                     gopts->get_variable_in_option<string>(gsystem_item, "factory", "ascii"),
		                     gopts->get_variable_in_option<string>(gsystem_item, "variation", "default"),
		                     gopts->get_variable_in_option<int>(gsystem_item, "runno", 1),
		                     gopts->get_variable_in_option<string>(gsystem_item, "annotations",
		                                                           UNINITIALIZEDSTRINGQUANTITY)
		                    );
	}

	return systems;
}


// method to return a vector of GModifier from the options
vector<GModifier> getModifiers(GOptions* gopts) {
	vector<GModifier> gmods;

	auto gmodifier_node = gopts->getOptionNode("gmodifier");

	for (auto gmodifier_item : gmodifier_node) {
		gmods.emplace_back(
		                   gopts->get_variable_in_option<string>(gmodifier_item, "name", goptions::NODFLT),
		                   gopts->get_variable_in_option<string>(gmodifier_item, "shift", GSYSTEMNOMODIFIER),
		                   gopts->get_variable_in_option<string>(gmodifier_item, "tilt", GSYSTEMNOMODIFIER),
		                   gopts->get_variable_in_option<bool>(gmodifier_item, "isPresent", true));
	}

	return gmods;
}


// returns array of options definitions
GOptions defineOptions() {
	GOptions goptions(GSYSTEM_LOGGER);

	// System
	string help;
	help = "A system definition includes the geometry location, factory and variation \n \n";
	help +=
		R"RAWS(Example: +gsystem={detector: "experiments/clas12/targets", factory: "TEXT", variation: "bonus"})RAWS";

	vector<GVariable> gsystem = {
		{"name", goptions::NODFLT, "system name (mandatory). For ascii factories, it may include the path to the file"},
		{"factory", "sqlite", "factory name. Possible choices: ascii, CAD, GDML, sqlite. Default is ascii"},
		{"variation", "default", "geometry variation, default is \"default\")"},
		{"runno", 1, "runno, default is 1)"},
		{"annotations", UNINITIALIZEDSTRINGQUANTITY, "optional system annotations. Examples: \"mats_only\" "}
	};
	goptions.defineOption("gsystem", "defines the group of volumes in a system", gsystem, help);

	// Modifier
	help = "The volume modifer can shift, tilt, or delete a volume from the gworld \n \n";
	help += R"RAWS(Example: +gmodifier={volume: "targetCell", tilt: "0*deg, 0*deg, -10*deg" })RAWS";

	vector<GVariable> gmodifier = {
		{"name", goptions::NODFLT, "volume name (optional)"},
		{"shift", GSYSTEMNOMODIFIER, "volume shift added to existing position"},
		{"tilt", GSYSTEMNOMODIFIER, "volume tilt added to existing rotation"},
		{"isPresent", true, "f set to false, remove volume from world i"}
	};
	goptions.defineOption("gmodifier", "modify volume existence or placement", gmodifier, help);

	help = "root volume definition. Default is: " + string(ROOTDEFINITION) + ". \n\n";
	help += "Command line Example: -root=\"G4Box 25*cm 24*cm 40*cm G4_WATER\"\n";
	help += "YAML file example: root: G4Box, 24*cm, 24*cm, 40*cm, G4_WATER\n";
	goptions.defineOption(GVariable(ROOTWORLDGVOLUMENAME, ROOTDEFINITION, "root volume definition"),
	                      help);

	// add sql option to define host or sqlite file
	help = "sqlite file or sql host. Default is: " + string(GSYSTEMSQLITETDEFAULTFILE) + ". \n\n";
	goptions.defineOption(GVariable("sql", GSYSTEMSQLITETDEFAULTFILE, "sql host or sqlite file"),
	                      help);

	// add the experiment option to define the experiment, common for all systems
	goptions.defineOption(GVariable("experiment", "examples", "Experiment selection"),
	                      "Each experiment have a subset of unique systems");

	// add run number options, common for all systems
	goptions.defineOption(GVariable("runno", 1, "run number"),
	                      "All systems share this  run number");

	return goptions;
}
}
