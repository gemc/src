// gemc
#include "gfactory.h"

// gfields
#include "gmagneto.h"
#include "gfield_options.h"

// guts
#include <gemc/guts/gutsConventions.h>

// CLHEP
#include <CLHEP/Units/SystemOfUnits.h>

// c++
#include <algorithm>
#include <cmath>
#include <exception>

// #include "G4TransportationManager.hh"
// #include "G4PropagatorInField.hh"

namespace {

bool is_unset_field_name(const std::string& name) {
	return name.empty() || name == UNINITIALIZEDSTRINGQUANTITY || name == "not provided";
}

} // namespace


GMagneto::GMagneto(const std::shared_ptr<GOptions>& gopts,
                   const std::set<std::string>&     required_fields) : GBase(gopts, GMAGNETO_LOGGER) {
	// Allocate the registries that will hold field objects and their corresponding managers.
	fields_map     = std::make_shared<gFieldMap>();
	fields_manager = std::make_shared<gFieldMgrMap>();

	// Factory manager responsible for loading plugins and instantiating objects.
	GManager gFieldManager(gopts);

	// Translate user configuration (options) into concrete field definitions.
	// TODO: this should be done in gemc instead and passed to gmagneto? could be kept here
	std::vector<GFieldDefinition> field_definition_array = gfields::get_GFieldDefinition(gopts);

	for (auto& field_definition : field_definition_array) {
		std::string name = field_definition.name;

		// When a filter is given, load only the fields that are actually used. Fields no volume
		// references (e.g. reset via -no_field) have their plugin and map skipped entirely.
		if (!required_fields.empty() && required_fields.find(name) == required_fields.end()) {
			log->info(1, "Field <", name, "> is not used by any volume: skipping plugin and map load.");
			continue;
		}

		log->info(1, field_definition);

		// Only create each named field once; repeated names are ignored by this map check.
		if (fields_map->find(name) == fields_map->end()) {
			// Load the plugin, instantiate the field object, and cache it by name.
			fields_map->emplace(name, gFieldManager.LoadAndRegisterObjectFromLibrary<GField>(
				field_definition.gfieldPluginName(), gopts));

			// Pass the configuration down to the concrete implementation so it can parse/cache parameters.
			fields_map->at(name)->load_field_definitions(field_definition);

			// Create and cache the Geant4 field manager responsible for stepping/chord finding.
			fields_manager->emplace(name, fields_map->at(name)->create_FieldManager());
		}
	}

	// TODO: add min and max steps
	//	G4TransportationManager::GetTransportationManager()->GetPropagatorInField()->SetLargestAcceptableStep(10);
}

std::shared_ptr<GField> GMagneto::initialize_magnetic_field(
    const std::shared_ptr<GOptions>& gopts, double& field_polarity,
    std::shared_ptr<GLogger> caller_log) {
	if (gopts == nullptr) { return nullptr; }

	if (gopts->doesOptionExist(NO_FIELD_OPTION)) {
		const std::string no_field_value = gopts->getScalarString(NO_FIELD_OPTION);
		if (no_field_value == NO_FIELD_ALL) {
			if (caller_log != nullptr) {
				caller_log->info(1, "Global field reset by -", NO_FIELD_OPTION, "=", NO_FIELD_ALL,
				                 ": direct field probes disabled.");
			}
			return nullptr;
		}
	}

	if (!gopts->doesOptionExist(GLOBAL_FIELD_OPTION)) { return nullptr; }

	const std::string field_name = gopts->getScalarString(GLOBAL_FIELD_OPTION);
	if (is_unset_field_name(field_name)) { return nullptr; }

	for (const auto& field_definition : gfields::get_GFieldDefinition(gopts)) {
		if (field_definition.name != field_name) { continue; }

		const auto torus_scale_it = field_definition.field_parameters.find("torus_scale");
		if (torus_scale_it != field_definition.field_parameters.end()) {
			try {
				field_polarity = std::stod(torus_scale_it->second) < 0.0 ? -1.0 : 1.0;
			} catch (const std::exception&) {
				if (caller_log != nullptr) {
					caller_log->warning("Could not parse torus_scale <", torus_scale_it->second,
					                    "> for field polarity; using +1.");
				}
				field_polarity = 1.0;
			}
		}
		break;
	}

	auto magneto = std::make_unique<GMagneto>(gopts, std::set<std::string>{field_name});
	if (magneto->isField(field_name)) {
		if (caller_log != nullptr) {
			caller_log->info(1, "Using magnetic field <", field_name,
			                 "> for direct probes with torus polarity ", field_polarity);
		}
		return magneto->getField(field_name);
	}

	if (caller_log != nullptr) {
		caller_log->warning("Global field <", field_name,
		                    "> is configured but was not available for direct probes.");
	}
	return nullptr;
}

double GMagneto::magnetic_field_magnitude_tesla(
    const std::shared_ptr<GField>& magnetic_field, const G4ThreeVector& position) {
	using namespace CLHEP;

	if (magnetic_field == nullptr) { return 0.0; }

	const double point[3] = {position.x(), position.y(), position.z()};
	double bfield[3] = {0.0, 0.0, 0.0};
	magnetic_field->GetFieldValue(point, bfield);

	return std::sqrt(bfield[0] * bfield[0] + bfield[1] * bfield[1] + bfield[2] * bfield[2]) / tesla;
}

std::vector<std::string> GMagneto::getFieldNames() const {
	std::vector<std::string> names;
	names.reserve(fields_map->size());
	for (const auto& [name, field] : *fields_map) { names.push_back(name); }
	std::sort(names.begin(), names.end());
	return names;
}
