// glibrary
#include "gfactory.h"

// gsystem
#include "gmagneto.h"

using namespace std;

GMagneto::GMagneto(GOptions *gopts) : GStateMessage(gopts, "GMagneto", "gfield") {

    gFieldMap = new map<string, GField *>;
    gFieldMgrMap = new map<string, G4FieldManager *>;

    GManager gFieldManager("GMagneto", verbosity);

    // TODO: this should be done in gemc instead and passed to gmagneto, same as gopts.
    vector <GFieldDefinition> field_definition_array = gfields::get_GFieldDefinition(gopts);

    for (auto &field_definition: field_definition_array) {
        string name = field_definition.name;

		if (verbosity >= GVERBOSITY_SUMMARY) { G4cout << field_definition << G4endl; }

        if (gFieldMap->find(name) == gFieldMap->end()) {
            (*gFieldMap)[name] = gFieldManager.LoadAndRegisterObjectFromLibrary<GField>( field_definition.gfieldPluginName());
            (*gFieldMap)[name]->load_field_definitions(field_definition);
            (*gFieldMgrMap)[name] = (*gFieldMap)[name]->create_FieldManager();
        }
    }

}
