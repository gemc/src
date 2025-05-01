#ifndef G4SCENE_PROPERTIES
#define G4SCENE_PROPERTIES 1

/**
 * \file G4SceneProperties.h
 * \brief Provides the G4SceneProperties class to initialize scene properties for the Geant4 UImanager.
 *
 * \mainpage G4 Scene Properties Module
 *
 * \section intro_sec Introduction
 * The G4SceneProperties class is used to configure scene properties such as
 * adding text annotations or labels to the Geant4 visualization. It reads configuration
 * options from a GOptions object and logs important information using a GLogger instance.
 *
 * \section details_sec Details
 * This module is intended to be part of the initialization process for Geant4 visualization.
 * The class provides a method to generate a list of scene text commands, which are then used
 * to add textual elements to the display.
 *
 * \section usage_sec Usage
 * Create an instance of G4SceneProperties by providing a pointer to a GOptions object.
 * Call the addSceneTexts() method with the same (or another) GOptions pointer to retrieve
 * a vector of text commands that can be applied to the Geant4 UImanager.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */

#include <string>
#include <vector>
#include "glogger.h"

class GOptions; // Forward declaration if not already included

/**
 * \class G4SceneProperties
 * \brief Initializes scene properties for the Geant4 visualization.
 *
 * The G4SceneProperties class provides helper functions to configure the scene in Geant4.
 * It offers the ability to generate text commands that add labels or annotations to the scene.
 * Configuration is obtained from a GOptions instance, and logging is performed using a GLogger.
 */
class G4SceneProperties {

public:
    /**
     * \brief Constructs a G4SceneProperties object.
     *
     * Initializes the scene properties based on configuration options. The constructor
     * uses the provided GOptions pointer to configure internal settings and logs the
     * initialization details.
     *
     * \param gopts Pointer to the GOptions object containing configuration parameters.
     */
    explicit G4SceneProperties(GOptions *gopts);

    /// Default destructor.
    ~G4SceneProperties() = default;

    /**
     * \brief Generates a vector of scene text commands.
     *
     * Reads scene text-related configuration from the provided GOptions object and
     * constructs a vector of strings. Each string represents a command (e.g., to add a text label)
     * that can be applied to the Geant4 UImanager.
     *
     * \param gopts Pointer to the GOptions object containing scene text configuration.
     * \return A vector of strings representing the scene text commands.
     */
    std::vector<std::string> addSceneTexts(GOptions *gopts);

private:
	std::shared_ptr<GLogger> log;        ///< Logger instance for logging messages.
};

#endif
