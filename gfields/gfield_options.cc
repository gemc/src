#include "gfield_options.h"
#include "gfieldConventions.h"

// gemc
#include "gutilities.h"
#include "gfactory_options.h"

// namespace to define options
namespace gfields {

// Build field definitions by reading the option tree and translating each entry into a GFieldDefinition (non-Doxygen summary).
std::vector<GFieldDefinition> get_GFieldDefinition(const std::shared_ptr<GOptions>& gopts) {
	std::vector<GFieldDefinition> gfield_defs;

	// Multipoles:
	// Each "gmultipoles" entry becomes one independently named field definition.
	auto gmultipoles_node = gopts->getOptionNode("gmultipoles");
	for (auto gmultipoles_item : gmultipoles_node) {
		GFieldDefinition gfield_def = GFieldDefinition();

		// Core identity and integration configuration.
		gfield_def.name                = gopts->get_variable_in_option<std::string>(gmultipoles_item, "name", goptions::NODFLT);
		gfield_def.integration_stepper = gopts->get_variable_in_option<std::string>(gmultipoles_item, "integration_stepper", GFIELD_DEFAULT_INTEGRATION_STEPPER);
		gfield_def.minimum_step        = gutilities::getG4Number(gopts->get_variable_in_option<std::string>(gmultipoles_item, "minimum_step", GFIELD_DEFAULT_MINIMUM_STEP));

		// Multipole parameters:
		// Values are stored as strings to preserve unit expressions and are parsed later by the concrete field.
		gfield_def.add_map_parameter("pole_number", gopts->get_variable_in_option<std::string>(gmultipoles_item, "pole_number", goptions::NODFLT));
		gfield_def.add_map_parameter("vx", gopts->get_variable_in_option<std::string>(gmultipoles_item, "vx", GFIELD_DEFAULT_VERTEX));
		gfield_def.add_map_parameter("vy", gopts->get_variable_in_option<std::string>(gmultipoles_item, "vy", GFIELD_DEFAULT_VERTEX));
		gfield_def.add_map_parameter("vz", gopts->get_variable_in_option<std::string>(gmultipoles_item, "vz", GFIELD_DEFAULT_VERTEX));
		gfield_def.add_map_parameter("rotation_angle", gopts->get_variable_in_option<std::string>(gmultipoles_item, "rotation_angle", GFIELD_DEFAULT_ROTANGLE));
		gfield_def.add_map_parameter("rotaxis", gopts->get_variable_in_option<std::string>(gmultipoles_item, "rotaxis", goptions::NODFLT));
		gfield_def.add_map_parameter("strength", gopts->get_variable_in_option<std::string>(gmultipoles_item, "strength", goptions::NODFLT));
		gfield_def.add_map_parameter("longitudinal", gopts->get_variable_in_option<std::string>(gmultipoles_item, "longitudinal", "false"));

		// The type field controls the shared-library plugin name through GFieldDefinition::gfieldPluginName().
		gfield_def.type = "multipoles";

		gfield_defs.push_back(gfield_def);
	}

	return gfield_defs;
}


// Define all options for this module, including plugin fields and logger integration (non-Doxygen summary).
GOptions defineOptions() {
	GOptions goptions(GFIELD_LOGGER);
	goptions += GOptions(GMAGNETO_LOGGER);
	goptions += GOptions(PLUGIN_LOGGER);

	std::string help;
	help                               = "Adds gmultipoles field(s) to the simulation \n ";
	std::vector<GVariable> gmultipoles = {
		{"name", goptions::NODFLT, "Field name (unique key used by GMagneto maps)"},
		{"integration_stepper", GFIELD_DEFAULT_INTEGRATION_STEPPER, "Geant4 integration stepper name (string)"},
		{"minimum_step", GFIELD_DEFAULT_MINIMUM_STEP, "Minimum step for the G4ChordFinder (Geant4 length units)"},
		{"pole_number", goptions::NODFLT, "Pole number (even integer >= 2): 2=dipole, 4=quadrupole, ..."},
		{"vx", GFIELD_DEFAULT_VERTEX, "Origin X component (Geant4 length units)"},
		{"vy", GFIELD_DEFAULT_VERTEX, "Origin Y component (Geant4 length units)"},
		{"vz", GFIELD_DEFAULT_VERTEX, "Origin Z component (Geant4 length units)"},
		{"rotation_angle", GFIELD_DEFAULT_ROTANGLE, "Roll rotation angle about rotaxis (Geant4 angle units)"},
		{"rotaxis", goptions::NODFLT, "Rotation/longitudinal axis: one of X, Y, Z"},
		{"strength", goptions::NODFLT, "Field strength in Tesla (defined at 1 m reference radius for multipoles)"},
		{"longitudinal", "false", "If true, return a uniform field aligned with rotaxis (solenoid-like)"}
	};
	goptions.defineOption("gmultipoles", "define the e.m. gmultipoles", gmultipoles, help);

	return goptions;
}
}
