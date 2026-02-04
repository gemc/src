// G4Dialog
#include "g4dialog.h"
#include "g4dialog_options.h" // Provides G4DIALOG_LOGGER constant and option definitions
#include "tabs/gcommands.h"

G4Dialog::G4Dialog(const std::shared_ptr<GOptions>& gopt, QWidget* parent) :
	GBase(gopt, G4DIALOG_LOGGER),
	QWidget(parent) {
	// Summary: Create the main tab widget, populate tabs, and install the primary layout.

	auto dialogTabs = new QTabWidget;

	// Tab: Geant4 commands browser/executor.
	dialogTabs->addTab(new G4Commands(this), "Geant4 Commands");

	// Main vertical layout: tabs fill the widget.
	auto* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(dialogTabs);
	setLayout(mainLayout);

	log->debug(NORMAL, "View and Utilities tabs added.");
}
