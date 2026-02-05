// gui
#include "gui.h"

// gemc
#include "g4display.h"
#include "g4dialog.h"
#include "dbselectView.h"
#include "gtree.h"

void GemcGUI::createRightContent(std::shared_ptr<GOptions> gopts,
								 GDetectorConstruction* dc,
								 GBoard* gb) {
	// The stacked widget hosts all right-side pages. The left button bar selects the current index.
	rightContent = new QStackedWidget;

	// Page order must match the left button bar order so indexes remain consistent.
	rightContent->addWidget(new G4Display(gopts));
	rightContent->addWidget(new G4Dialog(gopts, gb));
	rightContent->addWidget(new DBSelectView(gopts, dc));
	rightContent->addWidget(new GTree(gopts, dc->get_g4volumes_map()));

	// Default to the first page and update the left bar visual highlight accordingly.
	rightContent->setCurrentIndex(0);
	leftButtons->press_button(0);
}
