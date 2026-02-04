#pragma once

/**
 * \file g4SceneProperties.h
 * \brief Declaration of the \ref G4SceneProperties helper used to initialize Geant4 scene visualization.
 *
 * \ref G4SceneProperties provides convenience methods to generate Geant4 visualization commands for:
 * - creating/opening a viewer,
 * - applying camera defaults,
 * - configuring viewer precision,
 * - inserting optional scene text annotations.
 *
 * The generated commands are intended to be sent to \c G4UImanager by the caller.
 *
 */

// C++
#include <string>
#include <vector>

// gemc
#include "gbase.h"

// g4display
#include "g4display_options.h"

/**
 * \class G4SceneProperties
 * \brief Helper for constructing Geant4 visualization command sequences.
 *
 * \ref G4SceneProperties encapsulates the “policy” of how a scene is initialized from options:
 * - determine whether GUI mode is enabled (\c --gui),
 * - optionally enable DAWN output (\c --useDawn),
 * - open a viewer driver and apply initial camera/viewer settings,
 * - add optional text annotations configured via the \c g4text option.
 *
 * The helper does not apply commands directly; it returns a list of command strings so the caller can decide
 * when to execute them (typically right after the Geant4 visualization system is initialized).
 */
class G4SceneProperties : public GBase<G4SceneProperties>
{
public:
	/**
	 * \brief Construct a scene-properties helper bound to a set of options.
	 *
	 * \param gopts Shared options object used for module configuration and logging.
	 */
	explicit G4SceneProperties(const std::shared_ptr<GOptions>& gopts)
		: GBase(gopts, G4SCENE_LOGGER) {
	}

	/**
	 * \brief Build commands that insert configured text annotations into the scene.
	 *
	 * The \c g4text option is parsed into a list of g4display::G4SceneText objects, and then mapped to
	 * Geant4 commands of the form:
	 * - \c /vis/scene/add/text  (3D text that belongs to the scene),
	 * - \c /vis/scene/add/text2D (2D text, when the Z coordinate is explicitly provided).
	 *
	 * For each entry the method:
	 * - sets \c /vis/set/textColour,
	 * - issues the appropriate \c /vis/scene/add/... command with position and size,
	 * - restores the default text color by issuing \c /vis/set/textColour with no arguments.
	 *
	 * \param gopts Shared options object used to read text configuration.
	 * \return Vector of Geant4 command strings that insert the configured text into the current scene.
	 */
	std::vector<std::string> addSceneTexts(const std::shared_ptr<GOptions>& gopts);

	/**
	 * \brief Build the full command sequence for scene initialization.
	 *
	 * The returned command list typically includes:
	 * - scene creation (\c /vis/scene/create),
	 * - optional DAWN viewer setup (when \c useDawn is enabled),
	 * - viewer open and initial configuration (when \c gui is enabled),
	 * - insertion of scene texts (when configured),
	 * - camera direction and precision settings.
	 *
	 * \param gopts Shared options object used to derive viewer/camera configuration.
	 * \return Vector of Geant4 command strings for scene initialization.
	 */
	std::vector<std::string> scene_commands(const std::shared_ptr<GOptions>& gopts);
};
