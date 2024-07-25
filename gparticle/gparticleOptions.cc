// gparticle
#include "gparticleOptions.h"
#include "gparticleConventions.h"

#include <iostream>
using namespace std;

// namespace to define options
namespace gparticle {

    // method to return a vector of GParticles from the options
    vector <Gparticle> getGParticles(GOptions *gopts) {

        vector <Gparticle> gparticles;
        int verbosity = gopts->getVerbosityFor("gsystem");

        auto gparticle_node = gopts->get_option_node("gparticle");

        for (auto gparticle_item: gparticle_node) {
            gparticles.push_back(Gparticle(
                    gopts->get_variable_in_option<string>(gparticle_item, "name", goptions::NODFLT),
                    gopts->get_variable_in_option<int>(gparticle_item, "multiplicity", 1),
                    gopts->get_variable_in_option<float>(gparticle_item, "p", GPARTICLENOTDEFINED),
                    gopts->get_variable_in_option<float>(gparticle_item, "delta_p", 0),
                    gopts->get_variable_in_option<string>(gparticle_item, "punit", "MeV"),
                    gopts->get_variable_in_option<string>(gparticle_item, "randomMomentumModel", "uniform"),

                    gopts->get_variable_in_option<float>(gparticle_item, "theta", 0),
                    gopts->get_variable_in_option<float>(gparticle_item, "delta_theta", 0),
                    gopts->get_variable_in_option<string>(gparticle_item, "randomThetaModel", "uniform"),
                    gopts->get_variable_in_option<float>(gparticle_item, "phi", 0),
                    gopts->get_variable_in_option<float>(gparticle_item, "delta_phi", 0),
                    gopts->get_variable_in_option<string>(gparticle_item, "aunit", "deg"),

                    gopts->get_variable_in_option<float>(gparticle_item, "vx", 0),
                    gopts->get_variable_in_option<float>(gparticle_item, "vy", 0),
                    gopts->get_variable_in_option<float>(gparticle_item, "vz", 0),

                    gopts->get_variable_in_option<float>(gparticle_item, "delta_vx", 0),
                    gopts->get_variable_in_option<float>(gparticle_item, "delta_vy", 0),
                    gopts->get_variable_in_option<float>(gparticle_item, "delta_vz", 0),
                    gopts->get_variable_in_option<string>(gparticle_item, "vunit", "cm"),

                    gopts->get_variable_in_option<string>(gparticle_item, "randomVertexModel", "uniform"),
                    verbosity
            ));
        }

        return gparticles;
    }


    // returns array of options definitions
    GOptions defineOptions() {

        GOptions goptions;

        string help;
        help = "Adds a particle to the event generator \n ";
        help += "The particle is generated with a fixed or randomized momentum, angles, and vertex.  \n \n";
        help += "Examples: \n";
        help += "• 5 GeV electron along z: \n";
        help += "-gparticle=\"[{pname: e-, p: 5000}]\" \n \n";
        help = "•  three particles, one electron and two protons, identical except spread in theta: \n \n";
        help += "-gparticle=\"[{pname: e-, p: 2300, theta: 23.0}, {pname: proton, multiplicity: 2, p: 1200, theta: 14.0, delta_theta: 10}]\"\n";

        vector <GVariable> gparticle_v = {
                {"name",                goptions::NODFLT, "Particle name (mandatory),  for example \"proton\""},
                {"multiplicity",        1,                "How many copies of this particle will be generated in each event"},
                {"p",                   goptions::NODFLT, "Particle momentum"},
                {"delta_p",             0,                "Particle momentum range, centered on p."},
                {"punit",               "MeV",            "Geant4 Unit for the particle momentum. "},
                {"randomMomentumModel", "uniform",        "Momentum randomization. 'gaussian' (use deltas as sigmas)"},
                {"theta",               0,                "Particle polar angle. "},
                {"delta_theta",         0,                "Particle polar angle range, centered on theta. "},
                {"randomThetaModel",    "uniform",        "Distribute cos(theta) or theta. 'cosine': cos(theta) is uniform. 'uniform': theta is uniform"},
                {"phi",                 0,                "Particle azimuthal angle. "},
                {"delta_phi",           0,                "Particle azimuthal angle range, centered on phi. "},
                {"aunit",               "deg",            "Geant4 unit for the particle angles.  "},
                {"vx",                  0,                "Particle vertex x component. "},
                {"vy",                  0,                "Particle vertex y component. "},
                {"vz",                  0,                "Particle vertex z component. "},
                {"delta_vx",            0,                "Particle vertex range of the x component. "},
                {"delta_vy",            0,                "Particle vertex range of the y component. "},
                {"delta_vz",            0,                "Particle vertex range of the z component. "},
                {"vunit",               "cm",             "Unit for the particle vertex. "},
                {"randomVertexModel",   "uniform",        "Vertex randomization. Default: 'uniform'. Alternative: 'gaussian' (use deltas as sigmas), 'sphere'"}
        };

        goptions.defineOption("gparticle", "define the generator particle(s)", gparticle_v, help);

        return goptions;
    }

}
