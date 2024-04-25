// g4display
#include "g4displayOptions.h"
#include "g4displayConventions.h"

// full list from /vis/list
// ASCIITree ATree DAWNFILE G4HepRepFile HepRepFile RayTracer VRML2FILE gMocrenFile OpenGLImmediateQt OGLIQt OGLI OpenGLStoredQt OGLSQt OGL OGLS
vector<string> AVAILABLEG4VIEWERS = {
        "OpenGLImmediateQt",
        "OGLIQt",
        "OGLI",
        "OpenGLStoredQt",
        "OGLSQt",
        "OGL",
        "OGLS"};

// namespace to define options
namespace g4display {

    // JView
    void from_json(const json &j, JView &jview) {
        j.at("viewer").get_to(jview.viewer);
        j.at("dimension").get_to(jview.dimension);
        j.at("position").get_to(jview.position);
        j.at("segsPerCircle").get_to(jview.segsPerCircle);
    }

    // read structured option
    JView getJView(GOptions *gopts) {

        // getting json detector from option
        auto jview = gopts->getStructuredOptionAssignedValues("g4view");

        // projecting it onto GDetector structure
        return jview.front().get<JView>();
    }

    // JCamera
    void from_json(const json &j, JCamera &jcamera) {
        j.at("theta").get_to(jcamera.theta);
        j.at("phi").get_to(jcamera.phi);
    }

    // read structured option
    JCamera getJCamera(GOptions *gopts) {

        // getting json detector from option
        auto jcamera = gopts->getStructuredOptionAssignedValues("g4camera");

        // projecting it onto GDetector structure
        return jcamera.front().get<JCamera>();
    }

    // returns array of options definitions
    vector<GOption> defineOptions() {

        vector<GOption> goptions;


        // JView
        string VIEWERCHOICES = "g4 viewer. Available choices:\n\n";
        for (auto c: AVAILABLEG4VIEWERS) {
            VIEWERCHOICES += "\t\t\t\t- " + c + "\n";
        }

        // gview option, non groupable
        json jsonViewer = {
                {GNAME, "viewer"},
                {GDESC, VIEWERCHOICES},
                {GDFLT, GDEFAULTVIEWERDRIVER}
        };
        json jsonViewDimension = {
                {GNAME, "dimension"},
                {GDESC, "g4 viewer dimension"},
                {GDFLT, GDEFAULTVIEWERSIZE}
        };
        json jsonViewPosition = {
                {GNAME, "position"},
                {GDESC, "g4 viewer position"},
                {GDFLT, GDEFAULTVIEWERPOS}
        };
        json jsonSegsPerCircle = {
                {GNAME, "segsPerCircle"},
                {GDESC, "Number of segments per circle"},
                {GDFLT, GDEFAULTVSEGPERCIRCLE}
        };
        json jsonViewOption = {
                jsonViewer,
                jsonViewDimension,
                jsonViewPosition,
                jsonSegsPerCircle
        };

        vector<string> help;
        help.push_back("Defines the geant4 viewer properties:");
        help.push_back(" - screen dimensions");
        help.push_back(" - screen position");
        help.push_back(" - resolution in terms of segments per circle");
        help.push_back("");
        help.push_back("Example: -g4view={viewer: \"OGL\", dimension: \"1100x800\", position: \"+200+100\", segsPerCircle: 100}");

        // the last argument refers to "cumulative"
        goptions.push_back(GOption("g4view", "geant4 viewer properties", jsonViewOption, help, false));


        // JCamera
        json jsonCameraPhi = {
                {GNAME, "phi"},
                {GDESC, "geant4 camera phi"},
                {GDFLT, "0*deg"}
        };
        json jsonCameratheta = {
                {GNAME, "theta"},
                {GDESC, "geant4 camera theta"},
                {GDFLT, "0*deg"}
        };
        json jsonCameraOption = {
                jsonCameraPhi,
                jsonCameratheta
        };

        help.clear();
        help.push_back("Defines the geant4 camera view point");
        help.push_back("");
        help.push_back("Example: -g4camera={phi: \"20*deg\"; theta: \"15*deg\";}");

        // the last argument refers to "cumulative"
        goptions.push_back(GOption("g4camera", "geant4 camera", jsonCameraOption, help, false));

        // dawn switch
        goptions.push_back(GOption("dawn", "takes a screenshot of the loaded scene using the dawn driver"));

        // "gui" switch
        goptions.push_back(GOption("gui", "use Graphical User Interface"));

        // g4display verbosity
        json jsonMessageVerbosity = {
                {GNAME, "g4displayv"},
                {GDESC, "Verbosity for g4display. " + string(GVERBOSITY_DESCRIPTION)},
                {GDFLT, 0}
        };
        goptions.push_back(GOption(jsonMessageVerbosity));

        goptions += addSceneTextsOptions();
        goptions += addViewTextsOptions();


        return goptions;
    }

}
