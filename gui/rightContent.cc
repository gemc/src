// gui
#include "gui.h"

// gemc
#include "g4display.h"
#include "g4dialog.h"
#include "dbselectView.h"

void GemcGUI::createRightContent(std::shared_ptr<GOptions>              gopts,
                                GDetectorConstruction* dc) {
	rightContent = new QStackedWidget;

	rightContent->addWidget(new G4Display(gopts));
	rightContent->addWidget(new G4Dialog(gopts));
	rightContent->addWidget(new DBSelectView(gopts, dc));

	// set content and press the button to highlight it
	rightContent->setCurrentIndex(0);
	leftButtons->press_button(0);
}
