#pragma once

/**
 * \file g4displayutilities.h
 * \brief Declaration of the G4DisplayUtilities widget.
 *
 * \mainpage G4 Display Utilities Module
 *
 * \section intro_sec Introduction
 * The G4DisplayUtilities module provides a Qt widget that encapsulates various
 * display-related utilities for interacting with the Geant4 UI manager. This widget
 * can be used to control or display visualization parameters within a Geant4-based simulation.
 *
 * \section details_sec Details
 * The G4DisplayUtilities class inherits from QWidget and uses Qt's parent-child mechanism
 * to manage its child widgets automatically. It also logs messages using a GLogger instance.
 *
 * \section usage_sec Usage
 * To use this widget, create an instance by providing pointers to a GOptions object and a
 * GLogger instance. The widget does not require manual deletion of its child widgets, as
 * Qt handles this automatically through the parent-child relationship.
 *
* \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */

#include <QtWidgets>      // Includes all necessary Qt widget headers
#include "glogger.h"      // GLogger for logging messages
#include "G4UImanager.hh" // Geant4 UI Manager

/**
 * \class G4DisplayUtilities
 * \brief A utility widget for Geant4 display functionalities.
 *
 * G4DisplayUtilities provides a container for display utilities that interact with
 * Geant4's UImanager, making it easier to control and manage visualization settings.
 */
class G4DisplayUtilities : public QWidget {
    Q_OBJECT

public:
    /**
     * \brief Constructs a G4DisplayUtilities widget.
     *
     * This constructor initializes the widget with configuration options and a logger.
     * The widget's parent is set via the QWidget base class, ensuring that all child widgets
     * are automatically managed.
     *
     * \param gopt Pointer to the GOptions object used for configuration.
     * \param logger Pointer to the shared GLogger instance for logging.
     * \param parent Optional parent QWidget.
     */
    G4DisplayUtilities(GOptions* gopt, std::shared_ptr<GLogger> logger, QWidget* parent = nullptr);

    /**
     * \brief Destructor.
     *
     * Logs a debug message upon destruction. Child widgets are automatically deleted by Qt.
     */
    ~G4DisplayUtilities() {
	    log->debug(DESTRUCTOR, "G4DisplayUtilities");
    }

private:
	std::shared_ptr<GLogger> log;        ///< Logger instance for logging messages.
};

