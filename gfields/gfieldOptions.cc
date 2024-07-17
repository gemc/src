#include "gfieldOptions.h"
#include "gfieldConventions.h"

// namespace to define options
namespace gfield {

    vector <GField_MultipolesFactory> get_GField_MultipolesFactories(GOptions *gopts) {
        vector <GField_MultipolesFactory> gmultipoles;

        int verbosity = gopts->getVerbosityFor("gfields");

        auto gmultipoles_node = gopts->get_option_node("gmultipoles");

        for (auto gmultipoles_item: gmultipoles_node) {
            gmultipoles.push_back(GField_MultipolesFactory(
                    gopts->get_variable_in_option<string>(gmultipoles_item, "name", goptions::NODFLT),
                    gopts->get_variable_in_option<string>(gmultipoles_item, "integration_stepper", GFIELD_DEFAULT_INTEGRATION_STEPPER),
                    gopts->get_variable_in_option<float>(gmultipoles_item, "minimum_step", 1 * mm),
                    gopts->get_variable_in_option<int>(gmultipoles_item, "pole_number", 1),
                    gopts->get_variable_in_option<float>(gmultipoles_item, "vx", 0.0),
                    gopts->get_variable_in_option<float>(gmultipoles_item, "vy", 0.0),
                    gopts->get_variable_in_option<float>(gmultipoles_item, "vz", 0.0),
                    gopts->get_variable_in_option<float>(gmultipoles_item, "rotation_angle", 0.0),
                    gopts->get_variable_in_option<string>(gmultipoles_item, "rotaxis", "Z"),
                    gopts->get_variable_in_option<float>(gmultipoles_item, "strength", 1.0)
            ));
        }


        return gmultipoles;
    }


    // returns array of options definitions
    GOptions defineOptions() {


        GOptions goptions;

        string help;
        help = "Adds gmultipoles field(s) to the simulation \n ";

        vector <GVariable> gmultipoles = {
                {"name",           goptions::NODFLT,                        "Field name"},
                {"integration_stepper", GFIELD_DEFAULT_INTEGRATION_STEPPER, "Integration stepper"},
                {"minimum_step",   "1*mm",                                  "Minimum step for the G4ChordFinder"},
                {"pole_number",    1,                                       "Pole numner"},
                {"vx",             0.0,                                     "x component of the origin of the multipole"},
                {"vy",             0.0,                                     "y component of the origin of the multipole"},
                {"vz",             0.0,                                     "z component of the origin of the multipole"},
                {"rotation_angle", 0.0,                                     "rotation angle of the multipole"},
                {"rotaxis",        "Z",                                     "rotation axis of the multipole"},
                {"strength",       1.0,                                     "strength of the multipole"},
        };

        goptions.defineOption("gmultipoles", "define the e.m. gmultipoles", gmultipoles, help);


        return goptions;
    }
}
