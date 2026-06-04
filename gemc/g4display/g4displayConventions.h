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

// Default viewer driver used by the module.
#define GDEFAULTVIEWERDRIVER  "TOOLSSG_QT_GLES"

// Default viewer window size (WIDTHxHEIGHT).
#define GDEFAULTVIEWERSIZE    "800x800"

// Default viewer window position (X+Y or -X+Y depending on window manager conventions).
#define GDEFAULTVIEWERPOS     "-400+100"

// Default circle segmentation resolution for curved primitives.
#define GDEFAULTVSEGPERCIRCLE 50

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
