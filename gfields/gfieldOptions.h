#ifndef  GFIELDOPTIONS_H
#define  GFIELDOPTIONS_H 1

// glibrary
#include "goptions.h"

namespace gfield {

    // field properties
    struct JField {
        string name;
        string factory;
        string integration_stepper;
        string map_interpolation_method;
        double minimum_step;
        int verbosity;
    };

    void from_json(const json &j, JField &gfield);

    // method to return a vector of JField from a structured option
    vector<JField> getJFields(GOptions *gopts);


    // Multipoles are a special case of fields
    struct JMultipole {
        string name;
        int pole_number;
        double vx, vy, vz;
        double rotation_angle;
        string rotaxis; // capital
        double strength;
        string field_unit;
        string integration_stepper;
        string map_interpolation_method;
        double minimum_step;
        int verbosity;
    };

    void from_json(const json &j, JMultipole &mpole);

    // method to return a vector of JMultipole from a structured option
    vector<JMultipole> getJMultipoles(GOptions *gopts);

    // returns array of options definitions
    vector<GOption> defineOptions();

}


#endif
