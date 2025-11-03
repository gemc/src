// gemc
#include "gui.h"

void GemcGUI::createLeftButtons() {
	std::vector<std::string> bicons;

	bicons.push_back(":/gui/images/buttons/display");
	bicons.push_back(":/gui/images/buttons/dialog");
	bicons.push_back(":/gui/images/buttons/setup");
	bicons.push_back(":/gui/images/buttons/tree");
	bicons.push_back(":/gui/images/buttons/dialog");
	bicons.push_back(":/gui/images/buttons/dialog");
	bicons.push_back(":/gui/images/buttons/dialog");


	leftButtons = new GQTButtonsWidget(96, 96, bicons);
}
