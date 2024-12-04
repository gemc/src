// glibrary
#include "gfactory.h"

// gsystem
#include "gmagneto.h"

using namespace std;

GMagneto::GMagneto(GOptions *gopts) : GStateMessage(gopts, "GMagneto", "gfield") {

    gFieldMap = new map<string, GField *>;
    gFieldMgrMap = new map<string, G4FieldManager *>;

    GManager gFieldManager("GMagneto", verbosity);
    string pluginPath = string(getenv("GEMC")) + "/lib/";

    // multipoles from joptions
    vector <GFieldDefinition> field_definition_array = gfields::get_GFieldDefinition(gopts);

    for (auto &field_definition: field_definition_array) {
        string name = field_definition.name;
        string factory = field_definition.gfieldPluginName();

        cout << field_definition << endl;

        string pluginName = pluginPath + factory;

        if (gFieldMap->find(name) == gFieldMap->end()) {
            (*gFieldMap)[name] = gFieldManager.LoadAndRegisterObjectFromLibrary<GField>(pluginName);
            (*gFieldMap)[name]->set_field_definitions(field_definition);
            (*gFieldMgrMap)[name] = (*gFieldMap)[name]->create_FieldManager();
        }
    }

}
