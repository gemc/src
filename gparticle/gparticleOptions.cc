// gparticle
#include "gparticleOptions.h"
#include "gparticleConventions.h"

// namespace to define options
namespace gparticle {


    // method to return a vector of GDetectors from a structured option
    vector <JParticle> getJParticles(GOptions *gopts) {

        vector <JParticle> jparticles;

        auto jpars = gopts->getStructuredOptionAssignedValues("gparticle");

        // looking over each of the vector<json> items
        for (const auto &jpar: jpars) {
            jparticles.push_back(jpar.get<JParticle>());
        }

        return jparticles;
    }


    // returns array of options definitions
    GOption defineOptions() {

        GOption goptions;

        string help;
        help = "Adds a particle to the event generator \n ";
        help += "The particle is generated with a fixed or randomized momentum, angles, and vertex.  \n \n";
        help += "Examples: \n";
        help += "• 5 GeV electron along z: \n";


        vector <GVariable> gparticle = {
                {"name",                goptions::NODFLT, "Particle name (mandatory),  for example \"proton\""},
                {"multiplicity",        1,                "How many copies of this particle will be generated in each event"},
                {"p",                   0,                "Particle momentum"},
                {"theta",               0,                "Particle polar angle. Default: 0"},
                {"phi",                 0,                "Particle azimuthal angle. Default: 0"},
                {"delta_p",             0,                "Particle momentum range, centered on p. Default: 0"},
                {"delta_theta",         0,                "Particle polar angle range, centered on theta. Default: 0"},
                {"delta_phi",           0,                "Particle azimuthal angle range, centered on phi. Default: 0"},
                {"randomMomentumModel", "uniform",        "Momentum randomization. Default: uniform distribution. 'gaussian': use deltas as sigmas"},
                {"thetaModel",          "ct",             "Distribute cos(theta) or theta. 'ct' (default): cosTheta is uniform. 'flat': theta is uniform"},
                {"punit",               "MeV",            "Unit for the particle momentum. Default: \"MeV\" "},
                {"aunit",               "deg",            "Unit for the particle angles. Default: \"deg\" "},
                {"vx",                  0,                "Particle vertex x component. Default: 0"},
                {"vy",                  0,                "Particle vertex y component. Default: 0"},
                {"vz",                  0,                "Particle vertex z component. Default: 0"},
                {"delta_vx",            0,                "Particle vertex range of the x component. Default: 0"},
                {"delta_vy",            0,                "Particle vertex range of the y component. Default: 0"},
                {"delta_vz",            0,                "Particle vertex range of the z component. Default: 0"},
                {"delta_VR",            0,                "Particle vertex is generated within a sphere of radius delta_R. Default: 0"},
                {"randomVertexModel",   "uniform",        "Vertex randomization. Default: uniform distribution. 'gaussian': use deltas as sigmas"},
                {"vunit",               "cm",             "Unit for the particle vertex. Default: \"cm\" "}

        };


        json jmulti = {
                {GNAME, "multiplicity"},
                {GDESC, "How many copies of this particle will be generated in each event"},
                {GDFLT, 1}
        };

        json jp = {
                {GNAME, "p"},
                {GDESC, "Particle momentum"},
                {GDFLT, 0}
        };

        json jtheta = {
                {GNAME, "theta"},
                {GDESC, "Particle polar angle. Default: 0"},
                {GDFLT, 0}
        };

        json jphi = {
                {GNAME, "phi"},
                {GDESC, "Particle azimuthal angle. Default: 0"},
                {GDFLT, 0}
        };

        json jdelta_p = {
                {GNAME, "delta_p"},
                {GDESC, "Particle momentum range, centered on p. Default: 0"},
                {GDFLT, 0}
        };

        json jdelta_theta = {
                {GNAME, "delta_theta"},
                {GDESC, "Particle polar angle range, centered on theta. Default: 0"},
                {GDFLT, 0}
        };

        json jdelta_phi = {
                {GNAME, "delta_phi"},
                {GDESC, "Particle azimuthal angle range, centered on phi. Default: 0"},
                {GDFLT, 0}
        };

        json jthetaModel = {
                {GNAME, "thetaModel"},
                {GDESC, "Distribute cos(theta) or theta. 'ct' (default): cosTheta is uniform. 'flat': theta is uniform"},
                {GDFLT, "ct"}
        };

        json jrandomMomentumModel = {
                {GNAME, "randomMomentumModel"},
                {GDESC, "Momentum randomization. Default: uniform distribution. 'gaussian': use deltas as sigmas"},
                {GDFLT, "uniform"}
        };

        json jpunit = {
                {GNAME, "punit"},
                {GDESC, "Unit for the particle momentum. Default: \"MeV\" "},
                {GDFLT, "MeV"}
        };

        json jaunit = {
                {GNAME, "aunit"},
                {GDESC, "Unit for the particle angles. Default: \"deg\" "},
                {GDFLT, "deg"}
        };

        json jvx = {
                {GNAME, "vx"},
                {GDESC, "Particle vertex x component. Default: 0"},
                {GDFLT, 0}
        };

        json jvy = {
                {GNAME, "vy"},
                {GDESC, "Particle vertex y component. Default: 0"},
                {GDFLT, 0}
        };

        json jvz = {
                {GNAME, "vz"},
                {GDESC, "Particle vertex z component. Default: 0"},
                {GDFLT, 0}
        };

        json jdelta_vx = {
                {GNAME, "delta_vx"},
                {GDESC, "Particle vertex range of the x component. Default: 0"},
                {GDFLT, 0}
        };

        json jdelta_vy = {
                {GNAME, "delta_vy"},
                {GDESC, "Particle vertex range of the y component. Default: 0"},
                {GDFLT, 0}
        };

        json jdelta_vz = {
                {GNAME, "delta_vz"},
                {GDESC, "Particle vertex range of the z component. Default: 0"},
                {GDFLT, 0}
        };

        json jdelta_VR = {
                {GNAME, "delta_VR"},
                {GDESC, "Particle vertex is generated within a sphere of radius delta_R. Default: 0"},
                {GDFLT, 0}
        };

        json jrandomVertexModel = {
                {GNAME, "randomVertexModel"},
                {GDESC, "Vertex randomization. Default: uniform distribution. 'gaussian': use deltas as sigmas"},
                {GDFLT, "uniform"}
        };

        json jvunit = {
                {GNAME, "vunit"},
                {GDESC, "Unit for the particle vertex. Default: \"mm\" "},
                {GDFLT, "cm"}
        };

        json jparticleOption = {
                jpname,
                jmulti,
                jp,
                jtheta,
                jphi,
                jdelta_p,
                jdelta_theta,
                jdelta_phi,
                jrandomMomentumModel,
                jthetaModel,
                jpunit,
                jaunit,
                jvx,
                jvy,
                jvz,
                jdelta_vx,
                jdelta_vy,
                jdelta_vz,
                jdelta_VR,
                jrandomVertexModel,
                jvunit
        };

        vector <string> help;
        help.push_back("");
        help.push_back("Examples");
        help.push_back("");
        help.push_back("• 5 GeV electron along z:");
        help.push_back("  +gparticle={\"pname\": \"e-\"; \"p\": 5000;}");
        help.push_back("");
        help.push_back("• a 500 MeV neutron at theta=20 deg and uniform distribution in phi:");
        help.push_back("  +gparticle={\"pname\": \"neutron\"; \"p\": 500; \"theta\": 20; \"delta_phi\": 180}");
        help.push_back("");
        help.push_back("• 150 2.1 GeV electrons at theta=3deg, uniform in phi, at z=-2mm");
        help.push_back(
                "  +gparticle={ \"pname\": \"e-\", \"multiplicity\": 150, \"p\": 2100, \"theta\": 3.0, \"delta_phi\": 180.0, \"vz\": -2.0}");
        help.push_back("");
        help.push_back("• 250 3 GeV pions+ at theta between 5 and 15 deg (uniform in cos(theta)), phi = 180");
        help.push_back(
                "  +gparticle={ \"pname\": \"pi+\", \"multiplicity\": 250, \"p\": 3000, \"theta\": 10.0, \"delta_theta\": 5.0, \"phi\": 180}");
        help.push_back("");
        help.push_back("• 250 3 GeV pions+ at theta between 5 and 15 deg (uniform in theta), phi = 180");
        help.push_back(
                "  +gparticle={ \"pname\": \"pi+\", \"multiplicity\": 250, \"p\": 3000, \"theta\": 10.0, \"delta_theta\": 5.0, \"phi\": 180, \"thetaModel\": \"flat\"}");
        help.push_back("");
        help.push_back("• 400 MeV protons at theta=90deg, uniform in phi, v on a sphere of radius 0.5mm at vz=-4mm");
        help.push_back(
                "  +gparticle={ \"pname\": \"proton\", \"multiplicity\": 400, \"p\": 150, \"theta\": 90.0, \"delta_phi\": 180.0, \"delta_VR\": 0.5, \"vz\": -4.0}");


        // the last argument refers to "cumulative"
        goptions.push_back(GOption("gparticle", "adds a particle to the event generator", jparticleOption, help, true));


        return goptions;
    }

}
