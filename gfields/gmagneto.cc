// glibrary
#include "gfactory.h"

// gsystem
#include "gmagneto.h"

using namespace std;

GMagneto::GMagneto(GOptions *gopts) : GStateMessage(gopts, "GMagneto", "verbosity") {

    gFieldMap = new map<string, GField *>;
    gFieldMgrMap = new map<string, G4FieldManager *>;

    GManager gFieldManager("GMagneto", verbosity);
    string pluginPath = string(getenv("GEMC")) + "/lib/";


    // multipoles from joptions
    vector <GFieldDefinition> field_definitions = gfield::get_GFieldDefinition(gopts);

    for (auto &field_definition: field_definitions) {


//        string name = gmultipole.get_name();
//        string factory = string(GFIELD_MULTIPOLES_FACTORY_NAME);
//
//        string pluginName = pluginPath + factory;

//        (*gFieldMgrMap)[name] = (*gFieldMap)[name]->create_FieldManager(gmultipole.get_stepper(), gmultipole.get_min_step());
    }
}
