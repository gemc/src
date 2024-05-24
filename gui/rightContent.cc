// gemc
#include "gui.h"
#include "g4controls.h"

void GemcGUI::createRightContent(GOptions *gopts) {

    rightContent = new QStackedWidget;

    rightContent->addWidget(new G4Controls(gopts));

}
