// g4display
#include "g4displayOptions.h"
#include "g4displayConventions.h"



// namespace to define options
namespace g4display {

    // full list from /vis/list
    // TODO: lets get that programmatically
    // ASCIITree ATree DAWNFILE G4HepRepFile HepRepFile RayTracer VRML2FILE gMocrenFile OpenGLImmediateQt OGLIQt OGLI OpenGLStoredQt OGLSQt OGL OGLS
    vector <string> AVAILABLEG4VIEWERS = {
            "OpenGLImmediateQt",
            "OGLIQt",
            "OGLI",
            "OpenGLStoredQt",
            "OGLSQt",
            "OGL",
            "OGLS"
    };

//    // read structured option
//    JView getJView(GOptions *gopts) {
//
//        // getting json detector from option
//        auto jview = gopts->getStructuredOptionAssignedValues("g4view  \n ";
//
//        // projecting it onto GDetector structure
//        return jview.front().get<JView>();
//    }
//
//    // read structured option
//    JCamera getJCamera(GOptions *gopts) {
//
//        // getting json detector from option
//        auto jcamera = gopts->getStructuredOptionAssignedValues("g4camera  \n ";
//
//        // projecting it onto GDetector structure
//        return jcamera.front().get<JCamera>();
//    }

    // returns array of options definitions
    vector <GOption> defineOptions() {

        vector <GOption> goptions;
        string help;

        // JView
        string VIEWERCHOICES = "g4 viewer. Available choices:\n\n";
        for (auto c: AVAILABLEG4VIEWERS) {
            VIEWERCHOICES += "\t\t\t\t- " + c + "\n";
        }

        vector <GVariable> g4view = {
                {"driver",          string(GDEFAULTVIEWERDRIVER), "Geant4 vis driver"},
                {"dimension",       string(GDEFAULTVIEWERSIZE),   "g4 viewer dimension"},
                {"position",        string(GDEFAULTVIEWERPOS),    "g4 viewer position"},
                {"segsPerCircle",   GDEFAULTVSEGPERCIRCLE,           "Number of segments per circle"}

        };

        help = "Defines the geant4 viewer properties:  \n ";
        help += " - screen dimensions  \n ";
        help += " - screen position  \n ";
        help += " - resolution in terms of segments per circle  \n ";
        help += " Example: -g4view={viewer: \"OGL\", dimension: \"1100x800\", position: \"+200+100\", segsPerCircle: 100}  \n \n";
        help += "-g4view=\"[{pname: e-, multiplicity: 1, p: 2300, theta: 23.0}, {pname: proton, multiplicity: 2, p: 1200, theta: 14.0}]\"\n";

        goptions. defineOption("g4view", "Defines the geant4 viewer properties", g4view, help);





        // JCamera
//        json jsonCameraPhi = {
//                {GNAME, "phi"},
//                {GDESC, "geant4 camera phi"},
//                {GDFLT, "0*deg"}
//        };
//        json jsonCameratheta = {
//                {GNAME, "theta"},
//                {GDESC, "geant4 camera theta"},
//                {GDFLT, "0*deg"}
//        };
//        json jsonCameraOption = {
//                jsonCameraPhi,
//                jsonCameratheta
//        };

//        help.clear();
//        help "Defines the geant4 camera view point  \n ";
//        help "  \n ";
//        help "Example: -g4camera={phi: \"20*deg\"; theta: \"15*deg\";}  \n ";
//
//        // the last argument refers to "cumulative"
//        goptions GOption("g4camera", "geant4 camera", jsonCameraOption, help, false));
//
//        // dawn switch
//        goptions GOption("dawn", "takes a screenshot of the loaded scene using the dawn driver"));



//
//        goptions += addSceneTextsOptions();
//        goptions += addViewTextsOptions();


        return goptions;
    }

}
