#pragma once

/**
 * \file g4Text.h
 * \brief Scene text option structures and helpers for the g4display module.
 *
 * This header defines:
 * - g4display::G4SceneText : a small POD-like structure that describes a single text annotation.
 * - g4display::getSceneTexts : extracts configured texts from a GOptions option node.
 * - g4display::addSceneTextsOptions : defines the \c g4text option schema.
 *
 */

#include "g4displayConventions.h"

// gemc
#include "goptions.h"

namespace g4display {
/**
 * \struct G4SceneText
 * \brief One text annotation to be inserted into the Geant4 scene.
 *
 * Each configured item is converted into Geant4 visualization commands by \ref G4SceneProperties.
 *
 * Interpretation rules:
 * - \c kind = "2D" creates a \c /vis/scene/add/text2D command.
 * - \c kind = "3D" creates a \c /vis/scene/add/text command using \c x, \c y, \c z, \c unit, \c dx, and \c dy.
 */
struct G4SceneText
{
	/// Text string to be displayed.
	std::string text;

	/// Text color name understood by Geant4 (e.g. \c "black", \c "red").
	std::string color;

	/// Text kind: \c "2D" for text2D or \c "3D" for text attached in scene coordinates.
	std::string kind;

	/// Optional text layout such as \c "right"; empty keeps Geant4 default layout.
	std::string layout;

	/// X position.
	double x;

	/// Y position.
	double y;

	/// Z position used by 3D text.
	double z = GNOT_SPECIFIED_SCENE_TEXT_Z;

	/// Unit used by 3D text positions.
	std::string unit;

	/// Text size parameter passed to Geant4 visualization command.
	int size;

	/// 3D text X offset.
	double dx;

	/// 3D text Y offset.
	double dy;
};

/**
 * \brief Extract scene text entries from the \c g4text option node.
 *
 * The function reads the option node named \c "g4text" and projects each element into a \ref G4SceneText.
 * Missing properties are filled with defaults defined by the option schema.
 *
 * \param gopts Shared options object used to access the \c g4text option node.
 * \return A vector of \ref G4SceneText entries in the order they appear in the option node.
 */
std::vector<G4SceneText> getSceneTexts(const std::shared_ptr<GOptions>& gopts);

/**
 * \brief Define the \c g4text structured option schema.
 *
 * This helper returns a GOptions object that defines the \c g4text option. The option supports multiple text items,
 * each item providing:
 * - \c text (required)
 * - \c color (default \c "black")
 * - \c kind (default \c "2D"; use \c "3D" for \c /vis/scene/add/text)
 * - \c layout (default empty; e.g. \c "right")
 * - \c x, \c y (default \c 0)
 * - \c z (default sentinel \c GNOT_SPECIFIED_SCENE_TEXT_Z)
 * - \c unit (default \c "cm" for 3D text)
 * - \c size (default \c 24.0)
 * - \c dx, \c dy (default \c 4.0 for 3D text offsets)
 *
 * \return A GOptions object containing the \c g4text option definition.
 */
GOptions addSceneTextsOptions();
} // namespace g4display
