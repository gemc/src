#pragma once

/**
 * \file g4display.h
 * \brief Declaration of the \ref G4Display main widget.
 *
 * \ref G4Display is the top-level Qt widget for the g4display module. It hosts one or more tabs
 * that control Geant4 visualization through GUI interactions.
 *
 */

// C++
#include <memory>

// gemc
#include "gbase.h"

// qt
#include <QtWidgets/QWidget>

/**
 * \class G4Display
 * \brief Main GUI container for Geant4 visualization controls.
 *
 * G4Display:
 * - owns/uses the module logger (via its GBase inheritance),
 * - builds a \c QTabWidget containing visualization control panels (e.g. \ref G4DisplayView),
 * - is typically embedded into a higher-level application window.
 *
 * Ownership model:
 * - Tabs are created as child widgets of the \ref G4Display instance; Qt manages their lifetime.
 * - Copy is disabled to avoid unintended QWidget copying and logger ownership issues.
 */
class G4Display : public GBase<G4Display>, public QWidget
{
public:
	/**
	 * \brief Construct the main display widget.
	 *
	 * The constructor initializes the base logger for the \c "g4display" module and creates the tabbed UI.
	 *
	 * \param gopt   Shared options object used to configure the module and logging.
	 * \param parent Optional Qt parent widget for ownership management.
	 */
	explicit G4Display(const std::shared_ptr<GOptions>& gopt, QWidget* parent = nullptr);

	/// Copy is disabled (Qt widgets are not safely copyable and ownership should remain explicit).
	G4Display(const G4Display&) = delete;

	/// Assignment is disabled for the same reason as copy construction.
	G4Display& operator=(const G4Display&) = delete;
};
