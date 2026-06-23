#include "gfield_options.h"
#include "gfieldConventions.h"
#include "gmagneto.h"

// geant4
#include "G4UnitsTable.hh"

// gemc
#include "gutilities.h"
#include "gfactory_options.h"
#include "goptionsConventions.h"

// c++
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

namespace gfields {

namespace {

struct FieldQueryPoint {
	double      position[3] = {0.0, 0.0, 0.0};
	std::string source;
	int         line = 0;
};

bool is_query_set(const std::string& value) {
	return !value.empty() && value != UNINITIALIZEDSTRINGQUANTITY && value != "not provided";
}

FieldQueryPoint parse_field_query_point(const std::string& expression, const std::string& source, int line) {
	std::string cleaned = expression;
	if (const auto comment = cleaned.find('#'); comment != std::string::npos) {
		cleaned = cleaned.substr(0, comment);
	}
	for (auto& c : cleaned) {
		if (c == ',') { c = ' '; }
	}
	cleaned = gutilities::removeLeadingAndTrailingSpacesFromString(cleaned);

	FieldQueryPoint point;
	point.source = source;
	point.line   = line;

	if (cleaned.empty()) { return point; }

	std::istringstream tokens(cleaned);
	std::string        x;
	std::string        y;
	std::string        z;
	std::string        extra;
	tokens >> x >> y >> z >> extra;
	if (x.empty() || y.empty() || z.empty() || !extra.empty()) {
		std::cerr << FATALERRORL << "field query point must contain exactly three coordinates with units";
		if (line > 0) { std::cerr << " at " << source << ":" << line; }
		else { std::cerr << " in " << source; }
		std::cerr << ". Got <" << expression << ">." << std::endl;
		std::exit(EC__G4NUMBERERROR);
	}

	point.position[0] = gutilities::getG4Number(x, true);
	point.position[1] = gutilities::getG4Number(y, true);
	point.position[2] = gutilities::getG4Number(z, true);
	return point;
}

bool is_blank_or_comment_line(const std::string& line) {
	const auto trimmed = gutilities::removeLeadingAndTrailingSpacesFromString(line);
	return trimmed.empty() || trimmed[0] == '#';
}

void append_field_query_file_points(const std::string& filename, std::vector<FieldQueryPoint>& points) {
	std::ifstream input(filename);
	if (!input) {
		std::cerr << FATALERRORL << "can't open field query point file " << filename << "." << std::endl;
		std::exit(EC__FILENOTFOUND);
	}

	std::string line;
	int         line_number = 0;
	while (std::getline(input, line)) {
		++line_number;
		if (is_blank_or_comment_line(line)) { continue; }
		points.push_back(parse_field_query_point(line, filename, line_number));
	}
}

void print_field_query_result(const std::string& field_name,
                              const FieldQueryPoint& point,
                              const double bfield[3]) {
	const double bmag = std::sqrt(
		bfield[0] * bfield[0] +
		bfield[1] * bfield[1] +
		bfield[2] * bfield[2]);
	std::cout << "field=" << field_name
	          << " source=" << point.source;
	if (point.line > 0) { std::cout << ":" << point.line; }
	std::cout << " x=" << G4BestUnit(point.position[0], "Length")
	          << " y=" << G4BestUnit(point.position[1], "Length")
	          << " z=" << G4BestUnit(point.position[2], "Length")
	          << " Bx=" << G4BestUnit(bfield[0], "Magnetic flux density")
	          << " By=" << G4BestUnit(bfield[1], "Magnetic flux density")
	          << " Bz=" << G4BestUnit(bfield[2], "Magnetic flux density")
	          << " |B|=" << G4BestUnit(bmag, "Magnetic flux density")
	          << std::endl;
}

} // namespace

// Build field definitions by reading the option tree and translating each entry into a GFieldDefinition.
std::vector<GFieldDefinition> get_GFieldDefinition(const std::shared_ptr<GOptions>& gopts) {
	std::vector<GFieldDefinition> gfield_defs;

	// Multipoles:
	// Each "gmultipoles" entry becomes one independently named field definition.
	auto gmultipoles_node = gopts->getOptionNode("gmultipoles");
	for (auto gmultipoles_item : gmultipoles_node) {
		GFieldDefinition gfield_def = GFieldDefinition();

		// Core identity and integration configuration.
		gfield_def.name = gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "name", goptions::NODFLT);
		gfield_def.integration_stepper = gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "integration_stepper", GFIELD_DEFAULT_INTEGRATION_STEPPER);
		gfield_def.minimum_step = gutilities::getG4Number(gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "minimum_step", GFIELD_DEFAULT_MINIMUM_STEP));

		// Multipole parameters:
		// Values are stored as strings to preserve unit expressions and are parsed later by the concrete field.
		gfield_def.add_map_parameter("pole_number", gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "pole_number", goptions::NODFLT));
		gfield_def.add_map_parameter("vx", gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "vx", GFIELD_DEFAULT_VERTEX));
		gfield_def.add_map_parameter("vy", gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "vy", GFIELD_DEFAULT_VERTEX));
		gfield_def.add_map_parameter("vz", gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "vz", GFIELD_DEFAULT_VERTEX));
		gfield_def.add_map_parameter("rotation_angle", gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "rotation_angle", GFIELD_DEFAULT_ROTANGLE));
		gfield_def.add_map_parameter("rotaxis", gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "rotaxis", goptions::NODFLT));
		gfield_def.add_map_parameter("strength", gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "strength", goptions::NODFLT));
		gfield_def.add_map_parameter("longitudinal", gopts->get_variable_in_option<std::string>(
			gmultipoles_item, "longitudinal", "false"));

		// The type field controls the shared-library plugin name through GFieldDefinition::gfieldPluginName().
		gfield_def.type = "multipoles";

		gfield_defs.push_back(gfield_def);
	}

	// Generic plugin-backed fields:
	// Each "gfields" entry names a field, selects a plugin through "type", and carries an arbitrary
	// set of scalar parameters that are forwarded verbatim to the plugin via field_parameters. This
	// lets external plugins (e.g. clas12 mapped fields) be configured without changing this parser.
	auto gfields_node = gopts->getOptionNode("gfields");
	for (auto gfields_item : gfields_node) {
		GFieldDefinition gfield_def = GFieldDefinition();

		// Core identity and integration configuration (the schema-defined keys).
		gfield_def.name = gopts->get_variable_in_option<std::string>(
			gfields_item, "name", goptions::NODFLT);
		gfield_def.type = gopts->get_variable_in_option<std::string>(
			gfields_item, "type", goptions::NODFLT);
		gfield_def.integration_stepper = gopts->get_variable_in_option<std::string>(
			gfields_item, "integration_stepper", GFIELD_DEFAULT_INTEGRATION_STEPPER);
		gfield_def.minimum_step = gutilities::getG4Number(gopts->get_variable_in_option<std::string>(
			gfields_item, "minimum_step", GFIELD_DEFAULT_MINIMUM_STEP));

		// Every remaining (scalar) key is forwarded to the plugin as a string parameter.
		// Nested maps/sequences are not supported here: plugin parameters must be scalar values.
		for (auto it = gfields_item.begin(); it != gfields_item.end(); ++it) {
			auto key = it->first.as<std::string>();
			if (key == "name" || key == "type" || key == "integration_stepper" || key == "minimum_step") {
				continue;
			}
			gfield_def.add_map_parameter(key, it->second.as<std::string>());
		}

		gfield_defs.push_back(gfield_def);
	}

	return gfield_defs;
}


// Define all options for this module, including plugin fields and logger integration (non-Doxygen summary).
GOptions defineOptions() {
	GOptions goptions(GFIELD_LOGGER);
	goptions += GOptions(GMAGNETO_LOGGER);
	goptions += GOptions(PLUGIN_LOGGER);
	goptions += gfactory::defineOptions();

	std::string help;
	help = "Adds electromagnetic multipole field(s) to the simulation. \n \n";
	help += "Mandatory keys: name, pole_number, rotaxis, strength. \n \n";
	help += "Example (a quadrupole centered 30 cm downstream): \n";
	help += "-gmultipoles=\"[{name: q1, pole_number: 4, rotaxis: Z, strength: 1.2, vz: 30*cm}]\"\n";
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

	std::string gfields_help;
	gfields_help = "Adds a generic, plugin-backed electromagnetic field to the simulation. \n \n";
	gfields_help += "The 'type' selects the plugin shared library named gfield<type>Factory. \n";
	gfields_help += "Any additional scalar keys are forwarded verbatim to that plugin as string \n";
	gfields_help += "parameters (so the plugin alone decides which parameters it understands). \n \n";
	gfields_help += "Mandatory keys: name, type. \n \n";
	gfields_help += "Example (clas12 binary mapped field from the clas12-systems plugin): \n";
	gfields_help += "-gfields=\"[{name: clas12, type: clas12bin, solenoid: solenoid_map, torus: torus_map}]\"\n";
	std::vector<GVariable> gfields = {
		{"name", goptions::NODFLT, "Field name (unique key used by GMagneto maps)"},
		{"type", goptions::NODFLT, "Field type; selects the plugin shared library gfield<type>Factory"},
		{"integration_stepper", GFIELD_DEFAULT_INTEGRATION_STEPPER, "Geant4 integration stepper name (string)"},
		{"minimum_step", GFIELD_DEFAULT_MINIMUM_STEP, "Minimum step for the G4ChordFinder (Geant4 length units)"}
	};
	goptions.defineOption("gfields", "define a generic plugin-backed e.m. field", gfields, gfields_help);

	goptions.defineOption(
		GVariable(GLOBAL_FIELD_OPTION, UNINITIALIZEDSTRINGQUANTITY, "associate a field with the ROOT world volume"),
		"Associates a configured electromagnetic field with the ROOT (top-level) world volume.\n \n"
		"The value must be the name of a field defined with -gmultipoles or -gfields. The field's\n"
		"G4FieldManager is installed on the ROOT world volume and propagated to all daughters, so it\n"
		"applies everywhere a more specific per-volume field has not been set.\n \n"
		"Example: -global_field=dipole1\n \n");

	goptions.defineOption(
		GVariable(NO_FIELD_OPTION, UNINITIALIZEDSTRINGQUANTITY, "reset the field of one or more volumes"),
		"Removes the electromagnetic field association from one or more volumes.\n \n"
		"The value is either the name of a gvolume, a whitespace- or comma-separated list of gvolume\n"
		"names, or the special value 'all'. A listed volume that was associated with a field (per-volume\n"
		"or inherited) has that association removed, so it is left with no field. The special value 'all'\n"
		"resets every per-volume field and also clears the '" GLOBAL_FIELD_OPTION "' option.\n \n"
		"Fields that no volume uses as a result are not loaded: their plugins and field maps are skipped.\n \n"
		"Examples: -" NO_FIELD_OPTION "=target            (reset only the 'target' volume)\n"
		"          -" NO_FIELD_OPTION "=\"target, magnet\"  (reset both volumes)\n"
		"          -" NO_FIELD_OPTION "=all               (reset every field, including the global field)\n \n");

	goptions.defineOption(
		GVariable("fieldAt", UNINITIALIZEDSTRINGQUANTITY, "query all configured fields at x y z"),
		"Evaluate all configured electromagnetic fields at one absolute coordinate.\n \n"
		"The value must contain three coordinate expressions with units, separated by spaces.\n \n"
		"Example: -fieldAt=\"10*cm 0*mm 2*m\"\n \n");

	goptions.defineOption(
		GVariable("fieldMapPoints", UNINITIALIZEDSTRINGQUANTITY, "ASCII file of x y z points for field queries"),
		"Evaluate all configured electromagnetic fields at coordinates listed in an ASCII file.\n \n"
		"Each non-empty, non-comment line must contain three coordinate expressions with units.\n"
		"Coordinates may be separated by spaces or commas. Lines beginning with # are ignored.\n \n"
		"Example: -fieldMapPoints=points.txt\n \n");

	return goptions;
}

bool runFieldQueries(const std::shared_ptr<GOptions>& gopts) {
	const auto field_at         = gopts->getScalarString("fieldAt");
	const auto field_map_points = gopts->getScalarString("fieldMapPoints");

	if (!is_query_set(field_at) && !is_query_set(field_map_points)) { return false; }

	std::vector<FieldQueryPoint> points;
	if (is_query_set(field_at)) { points.push_back(parse_field_query_point(field_at, "fieldAt", 0)); }
	if (is_query_set(field_map_points)) { append_field_query_file_points(field_map_points, points); }

	auto magneto     = std::make_shared<GMagneto>(gopts);
	auto field_names = magneto->getFieldNames();
	if (field_names.empty()) {
		std::cerr << FATALERRORL << "field query requested, but no electromagnetic fields are configured."
		          << std::endl;
		std::exit(EC__NOOPTIONFOUND);
	}

	std::cout << "# field query results" << std::endl;
	for (const auto& point : points) {
		for (const auto& field_name : field_names) {
			double bfield[3] = {0.0, 0.0, 0.0};
			magneto->getField(field_name)->GetFieldValue(point.position, bfield);
			print_field_query_result(field_name, point, bfield);
		}
	}

	return true;
}

}
