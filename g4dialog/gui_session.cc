#include "gui_session.h"
#include "../g4dialog_options.h" // Provides G4DIALOG_LOGGER constant and option definitions
#include <iostream>

GUI_Session::GUI_Session(GOptions *gopt, GBoard* b) : board(b), log(new GLogger(gopt, G4DIALOG_LOGGER)) {

  	log->debug(CONSTRUCTOR, "GUI_Session");
	log->info(0, " g4 dialog : GUI_Session created");
}

// notice, we can't write to log inside ReceiveG4cout or ReceiveG4cerr
// as it will create a loop
G4int GUI_Session::ReceiveG4cout(const G4String &coutString) {
	if (board) {
		board->appendLog(QString::fromStdString(coutString));
	}
	return 0;
}

G4int GUI_Session::ReceiveG4cerr(const G4String &cerrString) {
	if (board) {
		board->appendLog(QString::fromStdString(cerrString));
	}
	return 0;
}
