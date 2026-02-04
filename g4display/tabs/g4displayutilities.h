#pragma once

/**
 * \file g4displayutilities.h
 * \brief Declaration of the \ref G4DisplayUtilities widget.
 *
 * This header declares a Qt widget used as an auxiliary “Utilities” tab in \ref G4Display.
 * The intent is to provide display-related helpers that do not belong in the main view-control tab.
 *
 */

#include <QtWidgets>      // Qt widgets (QWidget, layouts, etc.)
#include "glogger.h"      // Glogger (module logger)
#include "G4UImanager.hh" // Geant4 UI manager

/**
 * \class G4DisplayUtilities
 * \brief “Utilities” tab for \ref G4Display.
 *
 * This widget is placed in the “Utilities” tab of \ref G4Display. It is intended to host
 * miscellaneous visualization helpers such as:
 * - status/diagnostic panes (e.g. an in-GUI log board),
 * - quick command launchers for common viewer operations,
 * - future convenience widgets that do not fit in \ref G4DisplayView.
 *
 * Lifetime and ownership:
 * - Child widgets created with \c this as parent are owned by Qt and deleted automatically.
 * - Logging uses the shared Glogger instance passed by the parent \ref G4Display.
 */
class G4DisplayUtilities : public QWidget
{
	Q_OBJECT

public:
	/**
	 * \brief Construct the utilities tab widget.
	 *
	 * The constructor stores a shared logger and builds the tab contents.
	 *
	 * \param gopt   Shared options object used by the module (may be unused in early implementations).
	 * \param logger Shared logger instance (created/owned upstream, typically by \ref G4Display).
	 * \param parent Optional Qt parent widget for ownership management.
	 */
	G4DisplayUtilities(const std::shared_ptr<GOptions>& gopt,
	                   std::shared_ptr<GLogger>         logger,
	                   QWidget*                         parent = nullptr);

	/**
	 * \brief Destructor.
	 *
	 * Emits a debug-level log message. Child widgets are deleted by Qt parent-child ownership.
	 */
	~G4DisplayUtilities() override { log->debug(DESTRUCTOR, "G4DisplayUtilities"); }

private:
	/**
	 * \brief Shared logger for this widget.
	 *
	 * The logger is expected to remain valid for the lifetime of this widget.
	 */
	std::shared_ptr<GLogger> log;
};
