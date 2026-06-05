// gui
#include "gui.h"

// gemc
#include "g4display.h"
#include "g4displayutilities.h"
#include "g4dialog.h"
#include "dbselectView.h"
#include "gtree.h"

void GemcGUI::createRightContent(std::shared_ptr<GOptions> gopts,
								 GDetectorConstruction* dc,
								 GBoard* gb) {
	// The stacked widget hosts all right-side pages. The left button bar selects the current index.
	rightContent = new QStackedWidget;



	// Page order must match the left button bar order so indexes remain consistent.
	auto* setupView = new DBSelectView(gopts, dc);

	// display controls
	auto* g4display = new G4Display(gopts);
	rightContent->addWidget(g4display);
	auto* displayUtilities = g4display->findChild<G4DisplayUtilities*>();
	if (displayUtilities) {
		connect(displayUtilities, &G4DisplayUtilities::sceneOptionsChanged,
				this, &GemcGUI::refreshVisualizationFromOptions);
	}

	// setup/systems controls
	rightContent->addWidget(setupView);
	connect(setupView, &DBSelectView::geometryAboutToReload, this, &GemcGUI::resetVisualizationBeforeGeometryReload);
	connect(setupView, &DBSelectView::geometryReloaded, this, &GemcGUI::refreshGeometryTree);

	// volume controls
	geometryTree = new GTree(gopts, dc && dc->has_built_geometry()
	                                ? dc->get_g4volumes_map()
	                                : std::unordered_map<std::string, G4Volume*>{});
	rightContent->addWidget(geometryTree);

	// g4dialog
	rightContent->addWidget(new G4Dialog(gopts, gb));


	// Default to the first page and update the left bar visual highlight accordingly.
	rightContent->setCurrentIndex(0);
	leftButtons->press_button(0);
}
