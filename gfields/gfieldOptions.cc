#include "gfieldOptions.h"

// namespace to define options
namespace gfield {

    // projecting options onto JMultipole
    void from_json(const json &j, JMultipole &multipole) {
        j.at("pole_number").get_to(multipole.pole_number);

        j.at("vx").get_to(multipole.vx);
        j.at("vy").get_to(multipole.vy);
        j.at("vz").get_to(multipole.vz);

        j.at("rotation_angle").get_to(multipole.rotation_angle);
        string saxis = j.at("rotaxis");
        if (saxis == "x") multipole.rotaxis = 0;
        if (saxis == "Y") multipole.rotaxis = 1;
        if (saxis == "Z") multipole.rotaxis = 2;
        j.at("strength").get_to(multipole.strength);
        j.at("field_unit").get_to(multipole.field_unit);
    }

    // method to return a vector of JOutput from a structured option
    vector<JMultipole> getJMultipoles(GOptions *gopts) {

        vector<JMultipole> jpoles;

        auto gpoles = gopts->getStructuredOptionAssignedValues("gmultipole");

        // looking over each of the vector<json> items
        for (const auto &gpole: gpoles) {
            jpoles.push_back(gpole.get<JMultipole>());
        }

        return jpoles;
    }

    // returns array of options definitions
    vector<GOption> defineOptions() {

        vector<GOption> goptions;

        // two verbosity: one for events, one for streaming
        json jsonGFieldEventVerbosity = {
                {GNAME, "gfieldv"},
                {GDESC, "Verbosity for fields. " + string(GVERBOSITY_DESCRIPTION)},
                {GDFLT, 0}
        };
        goptions.push_back(GOption(jsonGFieldEventVerbosity));

        json jsonPole_Number = {
                {GNAME, "pole_number"},
                {GDESC, "Number of the multipole"},
                {GDFLT, 0}
        };

        json jsonPole_Vx = {
                {GNAME, "vx"},
                {GDESC, "x component of the origin of the multipole"},
                {GDFLT, 0.0}
        };
        json jsonPole_Vy = {
                {GNAME, "vy"},
                {GDESC, "y component of the origin of the multipole"},
                {GDFLT, 0.0}
        };
        json jsonPole_Vz = {
                {GNAME, "vz"},
                {GDESC, "z component of the origin of the multipole"},
                {GDFLT, 0.0}
        };
        json jsonPoleRotation = {
                {GNAME, "rotation_angle"},
                {GDESC, "rotation angle of the multipole"},
                {GDFLT, 0.0}
        };
        json jsonPoleRotaxis = {
                {GNAME, "rotaxis"},
                {GDESC, "rotation axis of the multipole"},
                {GDFLT, "Z"}
        };
        json jsonPoleStrength = {
                {GNAME, "strength"},
                {GDESC, "strength of the multipole"},
                {GDFLT, 1.0}
        };
        json jsonFieldUnit = {
                {GNAME, "field_unit"},
                {GDESC, "field unit"},
                {GDFLT, "tesla"}
        };

        json jsonMultipoles = {
                jsonPole_Number,
                jsonPole_Vx,
                jsonPole_Vy,
                jsonPole_Vz,
                jsonPoleRotation,
                jsonPoleRotaxis,
                jsonPoleStrength,
                jsonFieldUnit
        };

        vector<string> help;
        help.push_back("");
        help.push_back("Examples");
        help.push_back("");
        help.push_back("• 2T dipole :");
        help.push_back("  +gmultipole={\"strength\": \"2\"; \"pole_number\": 2;}");

        // the last argument refers to "cumulative"
        goptions.push_back(GOption("gmultipole", "Magnetic Multipole Definition", jsonMultipoles, help, true));

        return goptions;
    }
}
