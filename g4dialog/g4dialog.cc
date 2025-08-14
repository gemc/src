// G4Dialog
#include "g4dialog.h"
#include "g4dialog_options.h" // Provides G4DIALOG_LOGGER constant and option definitions
#include "tabs/gcommands.h"
#include "gui_session.h"

// geant4
#include "G4UImanager.hh"


G4Dialog::G4Dialog(const std::shared_ptr<GOptions>& gopt, QWidget* parent) :
	GBase(gopt, G4DIALOG_LOGGER),
	QWidget(parent) {
	auto dialogTabs = new QTabWidget;

	gboard   = new GBoard(gopt, this);
	auto UIM = G4UImanager::GetUIpointer();

	gui_session = std::make_unique<GUI_Session>(gopt, gboard);
	UIM->SetCoutDestination(gui_session.get());

	dialogTabs->addTab(new G4Commands(this), "Geant4 Commands");
	dialogTabs->addTab(gboard, "Log");

	auto* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(dialogTabs);
	setLayout(mainLayout);

	log->debug(NORMAL, "View and Utilities tabs added.");
}


G4Dialog::~G4Dialog() {

	// Detach G4 cout/cerr from our GUI session (avoid dangling callback)
	if (auto* UIM = G4UImanager::GetUIpointer()) {
		// If available in your G4 version, prefer checking current destination:
		// if (UIM->GetCoutDestination() == gui_session.get()) { ... }
		UIM->SetCoutDestination(nullptr);
	}

	// Destroy the session *now*, before member teardown sequence
	gui_session.reset();

}
