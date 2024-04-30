#ifndef  GFIELDOPTIONS_H
#define  GFIELDOPTIONS_H 1

// glibrary
#include "goptions.h"

namespace gfield {

    struct JMultipole {
        int pole_number;
        double vx, vy, vz;
        double rotation_angle;
        int rotaxis; // 0 for X, 1 for Y, 2 for Z
        double strength;
        string field_unit;
    };

    void from_json(const json& j, JMultipole& mpole);

    // method to return a vector of JMultipole from a structured option
    vector<JMultipole> getJMultipoles(GOptions *gopts);


    // returns array of options definitions
    vector<GOption> defineOptions();

}


#endif
