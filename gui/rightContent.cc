// gui
#include "gui.h"

// gemc
#include "g4display.h"

void GemcGUI::createRightContent(GOptions *gopts) {

    rightContent = new QStackedWidget;

    rightContent->addWidget(new G4Display(gopts));
}
