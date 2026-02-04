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
 * - If \c z is set to \c GNOT_SPECIFIED_SCENE_TEXT_Z, the text is treated as normal scene text (3D command).
 * - If \c z is explicitly provided, the text is treated as “2D text” (text2D command).
 */
struct G4SceneText
{
	/// Text string to be displayed.
	std::string text;

	/// Text color name understood by Geant4 (e.g. \c "black", \c "red").
	std::string color;

	/// X position.
	double x;

	/// Y position.
	double y;

	/// Z position; if left at default sentinel, Z is treated as “not specified”.
	double z = GNOT_SPECIFIED_SCENE_TEXT_Z;

	/// Text size parameter passed to Geant4 visualization command.
	int size;
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
 * - \c x, \c y (default \c 0)
 * - \c z (default sentinel \c GNOT_SPECIFIED_SCENE_TEXT_Z)
 * - \c size (default \c 24.0)
 *
 * \return A GOptions object containing the \c g4text option definition.
 */
GOptions addSceneTextsOptions();
} // namespace g4display
