#include "gfield_options.h"
#include "gfieldConventions.h"

// gemc
#include "gutilities.h"

// namespace to define options
namespace gfields {

std::vector<GFieldDefinition> get_GFieldDefinition(const std::shared_ptr<GOptions>& gopts) {
	std::vector<GFieldDefinition> gfield_defs;

	// multipoles
	auto gmultipoles_node = gopts->getOptionNode("gmultipoles");
	for (auto gmultipoles_item : gmultipoles_node) {
		GFieldDefinition gfield_def = GFieldDefinition();

		gfield_def.name                = gopts->get_variable_in_option<std::string>(gmultipoles_item, "name", goptions::NODFLT);
		gfield_def.integration_stepper = gopts->get_variable_in_option<std::string>(gmultipoles_item, "integration_stepper", GFIELD_DEFAULT_INTEGRATION_STEPPER);
		gfield_def.minimum_step        = gutilities::getG4Number(gopts->get_variable_in_option<std::string>(gmultipoles_item, "minimum_step", GFIELD_DEFAULT_MINIMUM_STEP));

		// if present, add remaining multipoles parameters
		gfield_def.add_map_parameter("pole_number", gopts->get_variable_in_option<std::string>(gmultipoles_item, "pole_number", GFIELD_DEFAULT_POLE_NUMBER));
		gfield_def.add_map_parameter("vx", gopts->get_variable_in_option<std::string>(gmultipoles_item, "vx", GFIELD_DEFAULT_VERTEX));
		gfield_def.add_map_parameter("vy", gopts->get_variable_in_option<std::string>(gmultipoles_item, "vy", GFIELD_DEFAULT_VERTEX));
		gfield_def.add_map_parameter("vz", gopts->get_variable_in_option<std::string>(gmultipoles_item, "vz", GFIELD_DEFAULT_VERTEX));
		gfield_def.add_map_parameter("rotation_angle", gopts->get_variable_in_option<std::string>(gmultipoles_item, "rotation_angle", GFIELD_DEFAULT_ROTANGLE));
		gfield_def.add_map_parameter("rotaxis", gopts->get_variable_in_option<std::string>(gmultipoles_item, "rotaxis", GFIELD_DEFAULT_ROTAXIS));
		gfield_def.add_map_parameter("strength", gopts->get_variable_in_option<std::string>(gmultipoles_item, "strength", GFIELD_DEFAULT_STRENGTH));
		gfield_def.type = "multipoles";
		gfield_defs.push_back(gfield_def);
	}

	return gfield_defs;
}


// returns array of options definitions
GOptions defineOptions() {
	GOptions goptions(GFIELD_LOGGER);

	std::string help;
	help                               = "Adds gmultipoles field(s) to the simulation \n ";
	std::vector<GVariable> gmultipoles = {
		{"name", goptions::NODFLT, "Field name"},
		{"integration_stepper", GFIELD_DEFAULT_INTEGRATION_STEPPER, "Integration stepper"},
		{"minimum_step", GFIELD_DEFAULT_MINIMUM_STEP, "Minimum step for the G4ChordFinder"},
		{"pole_number", GFIELD_DEFAULT_POLE_NUMBER, "Pole numner"},
		{"vx", GFIELD_DEFAULT_VERTEX, "x component of the origin of the multipole"},
		{"vy", GFIELD_DEFAULT_VERTEX, "y component of the origin of the multipole"},
		{"vz", GFIELD_DEFAULT_VERTEX, "z component of the origin of the multipole"},
		{"rotation_angle", GFIELD_DEFAULT_ROTANGLE, "rotation angle of the multipole"},
		{"rotaxis", GFIELD_DEFAULT_ROTAXIS, "rotation axis of the multipole"},
		{"strength", GFIELD_DEFAULT_STRENGTH, "strength of the multipole"}
	};
	goptions.defineOption("gmultipoles", "define the e.m. gmultipoles", gmultipoles, help);

	return goptions;
}
}
