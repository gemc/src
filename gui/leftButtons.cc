// gemc
#include "gui.h"

void GemcGUI::createLeftButtons() {
    vector <string> bicons;

    bicons.push_back(":/images/buttons/firstButton");
    bicons.push_back(":/images/buttons/secondButton");

    leftButtons = new GQTButtonsWidget(128, 128, bicons);
}
