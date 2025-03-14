// gui
#include "gui.h"

// gemc
#include "g4display.h"
#include "g4dialog.h"
#include "dbselectView.h"

void GemcGUI::createRightContent(GOptions *gopts) {

    rightContent = new QStackedWidget;

    rightContent->addWidget(new G4Display(gopts));
    rightContent->addWidget(new G4Dialog(gopts));
	rightContent->addWidget(new DBSelectView(gopts));

    // set content and press button to highlight it
    rightContent->setCurrentIndex(0);
    leftButtons->press_button(0);

}
