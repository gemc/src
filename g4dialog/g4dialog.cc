// G4Dialog
#include "g4dialog.h"
#include "g4dialog_options.h" // Provides G4DIALOG_LOGGER constant and option definitions
#include "tabs/gcommands.h"
#include "gui_session.h"

// geant4
#include "G4UImanager.hh"


G4Dialog::G4Dialog(const std::shared_ptr<GOptions>& gopt, QWidget *parent) : QWidget(parent), log(std::make_unique<GLogger>(gopt, G4DIALOG_LOGGER, "G4Dialog")) {

    log->debug(CONSTRUCTOR, "G4Dialog");

	auto dialogTabs = new QTabWidget;

	auto gboard = std::make_shared<GBoard>(gopt, this);
	auto UIM = G4UImanager::GetUIpointer();
	gui_session = std::make_unique<GUI_Session>(gopt, gboard);
	UIM->SetCoutDestination(gui_session.get());

	dialogTabs->addTab(new G4Commands(this), "Geant4 Commands");
	dialogTabs->addTab(gboard.get(), "Log");

	auto *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(dialogTabs);
	setLayout(mainLayout);

	log->debug(NORMAL, "View and Utilities tabs added.");

}
