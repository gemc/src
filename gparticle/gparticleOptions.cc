// gparticle
#include "gparticleOptions.h"
#include "gparticleConventions.h"

// namespace to define options
namespace gparticle {


    // JParticle
    // ------
    void from_json(const json &j, JParticle &jpar) {
        j.at("pname").get_to(jpar.pname);
        j.at("multiplicity").get_to(jpar.multiplicity);
        j.at("p").get_to(jpar.p);
        j.at("theta").get_to(jpar.theta);
        j.at("phi").get_to(jpar.phi);
        j.at("delta_p").get_to(jpar.delta_p);
        j.at("delta_theta").get_to(jpar.delta_theta);
        j.at("delta_phi").get_to(jpar.delta_phi);
        j.at("thetaModel").get_to(jpar.thetaModel);
        j.at("randomMomentumModel").get_to(jpar.randomMomentumModel);
        j.at("punit").get_to(jpar.punit);
        j.at("aunit").get_to(jpar.aunit);
        j.at("vx").get_to(jpar.vx);
        j.at("vy").get_to(jpar.vy);
        j.at("vz").get_to(jpar.vz);
        j.at("delta_vx").get_to(jpar.delta_vx);
        j.at("delta_vy").get_to(jpar.delta_vy);
        j.at("delta_vz").get_to(jpar.delta_vz);
        j.at("delta_VR").get_to(jpar.delta_VR);
        j.at("randomVertexModel").get_to(jpar.randomVertexModel);
        j.at("vunit").get_to(jpar.vunit);
    }

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
    vector <GOption> defineOptions() {

        vector <GOption> goptions;

        json jsonParticleVerbosity = {
                {GNAME, GPARTICLEVERBOSITY},
                {GDESC, "Verbosity for gparticle. " + string(GVERBOSITY_DESCRIPTION)},
                {GDFLT, 1}
        };
        goptions.push_back(GOption(jsonParticleVerbosity));


        // gparticle
        // ---------

        // gparticle is cumulative structured (groupable): can use -add
        json jpname = {
                {GNAME, "pname"},
                {GDESC, "Particle name, for example \"proton\""},
                {GDFLT, NODFLT}
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
