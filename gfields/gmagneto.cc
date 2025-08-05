// gemc
#include "gfactory.h"

// gfields
#include "gmagneto.h"
#include "gfield_options.h"

// #include "G4TransportationManager.hh"
// #include "G4PropagatorInField.hh"


GMagneto::GMagneto(std::shared_ptr<GOptions> gopts) : log(std::make_shared<GLogger>(gopts, GFIELD_LOGGER, "GMagneto")){

	fields_map = std::make_shared<gFieldMap>();
	fields_manager = std::make_shared<gFieldMgrMap>();

	log->debug(CONSTRUCTOR, "GMagneto");

	GManager gFieldManager(log, "GMagneto");

    // TODO: this should be done in gemc instead and passed to gmagneto? could be kept here
    std::vector <GFieldDefinition> field_definition_array = gfields::get_GFieldDefinition(gopts);

    for (auto &field_definition: field_definition_array) {
        std::string name = field_definition.name;
    	log->info(1, field_definition);

        if (fields_map->find(name) == fields_map->end()) {
            fields_map->emplace(name, gFieldManager.LoadAndRegisterObjectFromLibrary<GField>(field_definition.gfieldPluginName(), gopts));
        	fields_map->at(name)->load_field_definitions(field_definition);
        	fields_manager->emplace(name, fields_map->at(name)->create_FieldManager());
        }
    }

	// TODO: add min and max steps

//	G4TransportationManager::GetTransportationManager()->GetPropagatorInField()->SetLargestAcceptableStep(10);


}
