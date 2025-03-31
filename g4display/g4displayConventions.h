#ifndef  G4DISPLAY_H_CONVENTIONS
#define  G4DISPLAY_H_CONVENTIONS 1

// Notice: with qt6 we do not have access anymore to OGL, but the examples still do.
// This can be a potential problem in the future.

#include <vector>
#include <string>


#define GDEFAULTVIEWERDRIVER  "TOOLSSG_QT_GLES"
#define GDEFAULTVIEWERSIZE    "1200x1200"
#define GDEFAULTVIEWERPOS     "-400+100"
#define GDEFAULTVSEGPERCIRCLE 50
#define GNOT_SPECIFIED_SCENE_TEXT_Z (-1234.5)

#define EC__VISDRIVERNOTFOUND  1301

extern std::vector<std::string> AVAILABLEG4VIEWERS;

#endif
