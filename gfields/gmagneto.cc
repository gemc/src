// glibrary
#include "gfactory.h"

// gsystem
#include "gmagneto.h"

using namespace std;

GMagneto::GMagneto(GOptions *gopts) :
        GStateMessage(gopts, "GMagneto", "verbosity") {

    gFieldMap = new map<string, GField *>;
    gFieldMgrMap = new map<string, G4FieldManager *>;

    GManager gFieldManager("GMagneto", verbosity);
    string pluginPath = string(getenv("GEMC")) + "/lib/";

    // projecting json options onto vector of JField
    vector<gfield::JField> jfields = gfield::getJFields(gopts);

    // jfields from joptions
    for (auto &jfield: jfields) {

    }

    // multipoles from joptions
    vector<gfield::JMultipole> jmultipoles = gfield::getJMultipoles(gopts);

    for (auto &jmultipole: jmultipoles) {
        string name = jmultipole.name;
        string factory = string(GFIELD_MULTIPOLES_FACTORY_NAME);

        string pluginName = pluginPath + factory ;

        if (gFieldMap->find(name) == gFieldMap->end()) {
            (*gFieldMap)[name] = gFieldManager.LoadAndRegisterObjectFromLibrary<GField>(pluginName);

            // creating the map<string, string> to be passed to set_parameters
            map<string, string> parameters;
            parameters["name"] = name;
            parameters["factory"] = factory;
            parameters["integration_stepper"] = jmultipole.integration_stepper;
            parameters["map_interpolation_method"] = jmultipole.map_interpolation_method;
            parameters["minimum_step"] = to_string(jmultipole.minimum_step);
            parameters["pole_number"] = to_string(jmultipole.pole_number);
            parameters["vx"] = to_string(jmultipole.vx);
            parameters["vy"] = to_string(jmultipole.vy);
            parameters["vz"] = to_string(jmultipole.vz);
            parameters["rotation_angle"] = to_string(jmultipole.rotation_angle);
            parameters["rotaxis"] = jmultipole.rotaxis;
            parameters["strength"] = to_string(jmultipole.strength);
            parameters["field_unit"] = jmultipole.field_unit;
            (*gFieldMap)[name]->set_parameters(parameters);


            (*gFieldMgrMap)[name] = (*gFieldMap)[name]->create_FieldManager();
        }
    }

    // done with gFieldManager
    //gFieldManager.clearDLMap();
}
