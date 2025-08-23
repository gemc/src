// G4Dialog
#include "g4dialog.h"
#include "g4dialog_options.h" // Provides G4DIALOG_LOGGER constant and option definitions
#include "tabs/gcommands.h"



G4Dialog::G4Dialog(const std::shared_ptr<GOptions>& gopt, QWidget* parent) :
	GBase(gopt, G4DIALOG_LOGGER),
	QWidget(parent) {
	auto dialogTabs = new QTabWidget;

	dialogTabs->addTab(new G4Commands(this), "Geant4 Commands");

	auto* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(dialogTabs);
	setLayout(mainLayout);

	log->debug(NORMAL, "View and Utilities tabs added.");
}
