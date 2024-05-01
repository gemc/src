#ifndef  GFIELDOPTIONS_H
#define  GFIELDOPTIONS_H 1

// glibrary
#include "goptions.h"

namespace gfield {

    // field properties
    struct JFieldProperties {
        string field_name;
        string integration_stepper;
        string map_interpolation_method;
        double minimum_step;
        int verbosity;
    };

    void from_json(const json &j, JFieldProperties &field_properties);

    // method to return a vector of JFieldProperties from a structured option
    JFieldProperties getJFieldProperties(GOptions *gopts);


    // Multipoles
    struct JMultipole {
        string field_name;
        int pole_number;
        double vx, vy, vz;
        double rotation_angle;
        int rotaxis; // 0 for X, 1 for Y, 2 for Z
        double strength;
        string field_unit;
    };

    void from_json(const json &j, JMultipole &mpole);

    // method to return a vector of JMultipole from a structured option
    vector<JMultipole> getJMultipoles(GOptions *gopts);

    // returns array of options definitions
    vector<GOption> defineOptions();

}


#endif
