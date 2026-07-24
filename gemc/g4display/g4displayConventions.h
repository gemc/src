#pragma once

/**
 * \file g4displayConventions.h
 * \brief Conventions and constants used by the g4display module.
 *
 * This header centralizes default values and shared constants used by option definitions and helpers.
 * It also declares a global list of available Geant4 viewer drivers.
 *
 */

#include <vector>
#include <string>

// Notice: with Qt6 we do not have access anymore to OGL, but the examples still do.
// This can be a potential problem in the future.

// Viewer driver guide (availability of interactive drivers depends on the Geant4 build and platform):
// - ASCIITree (ATree): geometry hierarchy as text; useful headless, but provides no graphical view.
// - DAWNFILE (DAWNFILE): high-quality technical-renderer export; requires DAWN and is not interactive.
// - RayTracer (RT): software-rendered JPEG output; realistic but slower and not interactive.
// - VRML2FILE (VRML2FILE): portable 3D scene export; requires an external VRML viewer.
// - gMocrenFile (gMocrenFile): medical volume-rendering export; specialized and requires gMocren.
// - TOOLSSG_OFFSCREEN (TSG_OFFSCREEN, TSG_FILE): headless image/vector output; no interactive window.
// - OpenGLImmediateQt (OGLIQt, OGLI): Qt/OpenGL with low scene memory; complex scenes redraw slowly.
// - OpenGLStoredQt (OGLSQt, OGLS): Qt/OpenGL with fast redraw and picking; uses more graphics memory.
// - OpenGLImmediateX (OGLIX, OGLIQt_FALLBACK): X11/OpenGL with low scene memory; requires an X server.
// - OpenGLStoredX (OGLSX, OGLSQt_FALLBACK): fast X11/OpenGL redraw; requires X and more graphics memory.
// - RayTracerX (RTX): ray-traced JPEG plus an X window; requires X and remains computationally expensive.
// - TOOLSSG_X11_GLES (TSG_X11_GLES, TSGX11, TSG_QT_GLES_FALLBACK): interactive X11/GLES without Qt;
//   requires an X server and GLES support.

// Default viewer driver used by the module.
#define GDEFAULTVIEWERDRIVER  "OGLSQt"

// Default viewer window size (WIDTHxHEIGHT).
#define GDEFAULTVIEWERSIZE    "800x800"

// Default viewer window position (X+Y or -X+Y depending on window manager conventions).
#define GDEFAULTVIEWERPOS     "-400+100"

// Default circle segmentation resolution for curved primitives.
#define GDEFAULTVSEGPERCIRCLE 100

// Sentinel value used to indicate that a scene text Z coordinate was not specified.
#define GNOT_SPECIFIED_SCENE_TEXT_Z (-1234.5)

// Error codes (module-specific).
#define EC__VISDRIVERNOTFOUND  1301
#define EC__NOUIMANAGER        1302

/**
 * \brief List of Geant4 viewer driver names known to the application.
 *
 * The list is defined elsewhere (typically in a compilation unit) and used to validate
 * or present available driver choices.
 */
extern std::vector<std::string> AVAILABLEG4VIEWERS;
