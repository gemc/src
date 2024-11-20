// g4display
#include "g4display.h"
#include "tabs/g4displayview.h"
#include "tabs/g4displayutilities.h"

G4Display::G4Display(GOptions *gopt, QWidget *parent) : QTabWidget(parent) {

    setStyleSheet("QTabBar::tab       { background-color: #ACB6B6;}"
                  "QTabBar::tab:focus { color: #000011; }");

    addTab(new G4DisplayView(gopt, this), "View");
    addTab(new G4DisplayUtilities(gopt, this), "Utilities");
}
