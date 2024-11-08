// G4Dialog
#include "g4dialog.h"
#include "tabs/gcommands.h"

G4Dialog::G4Dialog(GOptions *gopt, QWidget *parent) : QTabWidget(parent) {
    setStyleSheet("QTabBar::tab       { background-color: #ACB6B6;}"
                  "QTabBar::tab:focus { color: #000011; }");


    addTab(new G4Commands(gopt, this), "Geant4 Commands");

}
