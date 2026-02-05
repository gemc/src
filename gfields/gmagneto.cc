// gemc
#include "gfactory.h"

// gfields
#include "gmagneto.h"
#include "gfield_options.h"

// #include "G4TransportationManager.hh"
// #include "G4PropagatorInField.hh"


GMagneto::GMagneto(const std::shared_ptr<GOptions>& gopts) : GBase(gopts, GMAGNETO_LOGGER) {
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
		log->info(1, field_definition);

		// Only create each named field once; repeated names are ignored by this map check.
		if (fields_map->find(name) == fields_map->end()) {
			// Load the plugin, instantiate the field object, and cache it by name.
			fields_map->emplace(name, gFieldManager.LoadAndRegisterObjectFromLibrary<GField>(field_definition.gfieldPluginName(), gopts));

			// Pass the configuration down to the concrete implementation so it can parse/cache parameters.
			fields_map->at(name)->load_field_definitions(field_definition);

			// Create and cache the Geant4 field manager responsible for stepping/chord finding.
			fields_manager->emplace(name, fields_map->at(name)->create_FieldManager());
		}
	}

	// TODO: add min and max steps
	//	G4TransportationManager::GetTransportationManager()->GetPropagatorInField()->SetLargestAcceptableStep(10);
}
