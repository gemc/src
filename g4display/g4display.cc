// g4display
#include "g4display.h"
#include "g4display_options.h"
#include "tabs/g4displayview.h"
#include "tabs/g4displayutilities.h"

G4Display::G4Display(GOptions* gopt, QWidget* parent) : QTabWidget(parent), log(new GLogger(gopt, G4DISPLAY_LOGGER)) {

	setStyleSheet("QTabBar::tab       { background-color: #ACB6B6;}"
	              "QTabBar::tab:focus { color: #000011; }");

	addTab(new G4DisplayView(gopt, log, this), "View");
	addTab(new G4DisplayUtilities(gopt, log, this), "Utilities");

}
