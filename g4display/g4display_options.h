#pragma once

/**
 * \file g4display_options.h
 * \brief Option structures and helpers for g4display configuration.
 *
 * This header defines lightweight structures that project portions of the GOptions configuration into
 * strongly-typed fields used by the GUI and by scene initialization helpers.
 *
 * The public API here is:
 * - \ref g4display::getG4View
 * - \ref g4display::getG4Camera
 * - \ref g4display::getG4Dawn
 * - \ref g4display::defineOptions
 *
 */

#include "goptions.h"

constexpr const char* G4DISPLAY_LOGGER = "g4display";
constexpr const char* G4SCENE_LOGGER   = "g4scene";

namespace g4display {
/**
 * \struct G4View
 * \brief Viewer configuration derived from the \c g4view option node.
 *
 * Fields map directly to a \c /vis/open command:
 * - \c driver : visualization driver name
 * - \c dimension : window size string (e.g. \c "800x800")
 * - \c position : window position string (e.g. \c "+200+100")
 * - \c segsPerCircle : circle segmentation precision for curved primitives
 */
struct G4View
{
	std::string driver;
	std::string dimension;
	std::string position;
	int         segsPerCircle;
};

/**
 * \brief Read the \c g4view option node and return a projected \ref G4View struct.
 *
 * \param gopts Shared options object to query.
 * \return \ref G4View populated from the \c g4view node.
 */
G4View getG4View(const std::shared_ptr<GOptions>& gopts);

/**
 * \struct G4Camera
 * \brief Camera angle configuration derived from the \c g4camera option node.
 *
 * The \c phi and \c theta strings are stored as provided in the option node and are typically parsed later
 * (e.g. into degrees for viewer commands).
 */
struct G4Camera
{
	std::string phi;
	std::string theta;
};

/**
 * \brief Read the \c g4camera option node and return a projected \ref G4Camera struct.
 *
 * \param gopts Shared options object to query.
 * \return \ref G4Camera populated from the \c g4camera node.
 */
G4Camera getG4Camera(const std::shared_ptr<GOptions>& gopts);

/**
 * \struct G4Dawn
 * \brief DAWN view configuration derived from the \c dawn option node.
 *
 * DAWN configuration is used when generating DAWNFILE output or screenshots.
 */
struct G4Dawn
{
	std::string phi;
	std::string theta;
};

/**
 * \brief Read the \c dawn option node and return a projected \ref G4Dawn struct.
 *
 * The implementation normalizes \c "null" values to \c goptions::NODFLT.
 *
 * \param gopts Shared options object to query.
 * \return \ref G4Dawn populated from the \c dawn node.
 */
G4Dawn getG4Dawn(const std::shared_ptr<GOptions>& gopts);

/**
 * \brief Define the full set of g4display options.
 *
 * This function builds a GOptions object for the module, including:
 * - \c g4view : viewer driver/window settings and precision.
 * - \c g4camera : initial camera angles.
 * - \c dawn / \c useDawn : DAWN view configuration and enable switch.
 * - \c g4text : optional scene text entries (added via g4display::addSceneTextsOptions).
 *
 * \return A GOptions instance containing the module option definitions.
 */
GOptions defineOptions();
} // namespace g4display
