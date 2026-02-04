#pragma once

// qt
#include <QtWidgets>

// gemc
#include "gbase.h"

/**
 * \class G4Dialog
 * \brief Main widget for the G4Dialog module.
 *
 * \details
 * G4Dialog is a tabbed Qt widget that aggregates UI utilities intended for interacting with
 * \c Geant4 from a GUI application. It derives from GBase (for logging/config integration)
 * and from \c QWidget (for embedding into Qt applications).
 *
 * Current tabs include:
 * - A \c Geant4 commands browser/executor (G4Commands).
 *
 * Ownership model:
 * - Tabs and layouts are created with Qt parent ownership, so they are released automatically
 *   when the parent widget is destroyed.
 */
class G4Dialog : public GBase<G4Dialog>, public QWidget
{
public:
	/**
	 * \brief Construct the main dialog widget.
	 *
	 * \param gopt Shared configuration/options object provided by the surrounding application.
	 * \param parent Parent widget for Qt ownership, or nullptr.
	 */
	explicit G4Dialog(const std::shared_ptr<GOptions>& gopt, QWidget* parent = nullptr);

	/// Destructor (defaulted).
	~G4Dialog() override = default;
};
