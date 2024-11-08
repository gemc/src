// gemc
#include "gui.h"

void GemcGUI::createLeftButtons() {
    vector <string> bicons;

    bicons.push_back(":/gui/images/buttons/display");
    bicons.push_back(":/gui/images/buttons/dialog");

    leftButtons = new GQTButtonsWidget(128, 128, bicons);
}
