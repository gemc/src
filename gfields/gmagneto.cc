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
    vector <GFieldDefinition> field_definitions = gfields::get_GFieldDefinition(gopts);

    for (auto &field_definition: field_definitions) {
        string name = field_definition.name;
        string factory = field_definition.gfieldPluginName();

        string pluginName = pluginPath + factory;

        cout << " HELLO " << name << " " << factory << " " << pluginName << endl;

//        // Load the plugin
//        gFieldManager.load_plugin(pluginName);
//
//        // Create the field
//        (*gFieldMap)[name] = gFieldManager.create_Field(name, field_definition);
    }


//        string name = gmultipole.get_name();
//        string factory = string(GFIELD_MULTIPOLES_FACTORY_NAME);
//
//        string pluginName = pluginPath + factory;

//        (*gFieldMgrMap)[name] = (*gFieldMap)[name]->create_FieldManager(gmultipole.get_stepper(), gmultipole.get_min_step());
//    }
}
