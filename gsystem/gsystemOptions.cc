// gsystem
#include "gsystemOptions.h"

// project goption to a system
namespace gsystem {

// method to return a vector of GSystem from the options
vector <GSystem> getSystems(GOptions *gopts) {

	vector <GSystem> systems;
	int verbosity = gopts->getVerbosityFor("gsystem");

	auto gsystem_node = gopts->getOptionNode("gsystem");

	for (auto gsystem_item: gsystem_node) {
		systems.push_back(GSystem(
				gopts->get_variable_in_option<string>(gsystem_item, "name", goptions::NODFLT),
				gopts->get_variable_in_option<string>(gsystem_item, "factory", "ascii"),
				gopts->get_variable_in_option<string>(gsystem_item, "variation", "default"),
				verbosity,
				gopts->get_variable_in_option<int>(gsystem_item, "runno", 1),
				gopts->get_variable_in_option<string>(gsystem_item, "annotations", UNINITIALIZEDSTRINGQUANTITY)
		));
	}

	return systems;
}


// method to return a vector of GModifier from the options
vector <GModifier> getModifiers(GOptions *gopts) {

	vector <GModifier> gmods;
	int verbosity = gopts->getVerbosityFor("gsystem");

	auto gmodifier_node = gopts->getOptionNode("gmodifier");

	for (auto gmodifier_item: gmodifier_node) {

		gmods.push_back(GModifier(
				gopts->get_variable_in_option<string>(gmodifier_item, "name", goptions::NODFLT),
				gopts->get_variable_in_option<string>(gmodifier_item, "shift", GSYSTEMNOMODIFIER),
				gopts->get_variable_in_option<string>(gmodifier_item, "tilt", GSYSTEMNOMODIFIER),
				gopts->get_variable_in_option<bool>(gmodifier_item, "isPresent", true),
				verbosity
		));
	}

	return gmods;
}


// returns array of options definitions
GOptions defineOptions() {
	GOptions goptions;

	// System
	string help;
	help = "A system definition includes the geometry location, factory and variation \n \n";
	help += "Example: +gsystem={detector: \"experiments/clas12/targets\", factory: \"TEXT\", variation: \"bonus\"}";

	vector <GVariable> gsystem = {
			{"name",      goptions::NODFLT,              "system name (mandatory). For ascii factories, it may include the path to the file"},
			{"factory",   "sqlite",                      "factory name. Possible choices: ascii, CAD, GDML, sqlite. Default is ascii"},
			{"variation", "default",                     "geometry variation, default is \"default\")"},
			{"runno",     1,                             "runno, default is 1)"},
			{"annotations", UNINITIALIZEDSTRINGQUANTITY, "optional system annotations. Examples: \"mats_only\" "}
	};
	goptions.defineOption("gsystem", "defines the group of volumes in a system", gsystem, help);

	// Modifier
	help = "The volume modifer can shift, tilt, or delete a volume from the gworld \n \n";
	help += "Example: +gmodifier={volume: \"targetCell\", tilt: \"0*deg, 0*deg, -10*deg\" }";

	vector <GVariable> gmodifier = {
			{"name",      goptions::NODFLT, "volume name (optional)"},
			{"shift", GSYSTEMNOMODIFIER,    "volume shift added to existing position"},
			{"tilt",  GSYSTEMNOMODIFIER,    "volume tilt added to existing rotation"},
			{"isPresent", true,             "f set to false, remove volume from world i"}
	};
	goptions.defineOption("gmodifier", "modify volume existence or placement", gmodifier, help);

	help = "root volume definition. Default is: " + string(ROOTDEFINITION) + ". \n\n";
	help += "Command line Example: -root=\"G4Box 25*cm 24*cm 40*cm G4_WATER\"\n";
	help += "YAML file example: root: G4Box, 24*cm, 24*cm, 40*cm, G4_WATER\n";
	goptions.defineOption(GVariable(ROOTWORLDGVOLUMENAME, ROOTDEFINITION, "root volume definition"),
						  help);

	// add sql option to define host or sqlite file
	goptions.defineOption(GVariable("sql", GSYSTEMSQLITETDEFAULTFILE, "sql host or sqlite file"),
						  "sql host or sqlite file");

	// add experiment option to define the experiment
	goptions.defineOption(GVariable("experiment", "examples", "Experiment selection"),
						  "Each experiment have a subset of unique systems");

	return goptions;
}
}
