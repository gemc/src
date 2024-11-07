// gemc
#include "gui.h"

void GemcGUI::createLeftButtons() {
    vector <string> bicons;

    bicons.push_back(":/gui/images/buttons/firstButton");
    bicons.push_back(":/gui/images/buttons/secondButton");
    bicons.push_back(":/gui/images/buttons/firstButton");
    bicons.push_back(":/gui/images/buttons/secondButton");
    bicons.push_back(":/gui/images/buttons/firstButton");
    bicons.push_back(":/gui/images/buttons/secondButton");
    bicons.push_back(":/gui/images/buttons/firstButton");

    leftButtons = new GQTButtonsWidget(64, 64, bicons);


}
