// g4displayutilities.cc
//
// Implementation of the G4DisplayUtilities tab.
// Doxygen documentation for this class is authoritative in g4displayutilities.h (see rule 7).

#include "g4displayutilities.h"

// c++
#include <string>
using namespace std;

G4DisplayUtilities::G4DisplayUtilities([[maybe_unused]] const std::shared_ptr<GOptions>& gopt,
                                       std::shared_ptr<GLogger>                          logger,
                                       QWidget*                                          parent)
	: QWidget(parent), log(logger) {
	log->debug(CONSTRUCTOR, "G4DisplayUtilities");

	// Currently this tab provides a minimal “log board” placeholder.
	// Future expansions may include:
	// - routing Glogger output to a QTextEdit
	// - quick buttons for common viewer commands
	// - visualization diagnostics and snapshots
	auto logTextEdit = new QTextEdit(this);
	logTextEdit->setReadOnly(true);
	logTextEdit->setText("Log Board Initialized");
	logTextEdit->setStyleSheet("background-color: lightblue;");
	logTextEdit->setMinimumHeight(200);
	logTextEdit->setMinimumWidth(400);

	// NOTE: This placeholder widget is not yet placed into a layout.
	// In a future functional expansion, a layout should be set and widgets added to it.
}
