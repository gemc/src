// gui
#include "gui.h"

// gemc
#include "g4display.h"
#include "g4displayutilities.h"
#include "g4dialog.h"
#include "dbselectView.h"
#include "gtree.h"
#include "pmakerView.h"
#include "gemc/actions/gaction.h"

// geant4
#include "G4RunManager.hh"

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

	// gvolume controls
	geometryTree = new GTree(gopts,
	                         dc && dc->has_built_geometry()
	                             ? dc->get_g4volumes_map()
	                             : std::unordered_map<std::string, G4Volume*>{},
	                         dc && dc->has_built_geometry()
	                             ? dc->get_gvolumes_flat_map()
	                             : std::unordered_map<std::string, const GVolume*>{});
	rightContent->addWidget(geometryTree);

	// g4dialog
	rightContent->addWidget(new G4Dialog(gopts, gb));

	// generator / particle maker — share the exact same GparticlePtr objects as the generator
	std::shared_ptr<std::vector<GparticlePtr>> sharedParticles;
	if (auto* rm = G4RunManager::GetRunManager()) {
		if (auto* ga = dynamic_cast<GAction*>(
		        const_cast<G4VUserActionInitialization*>(rm->GetUserActionInitialization())))
			sharedParticles = ga->getSharedParticles();
	}
	rightContent->addWidget(new PmakerView(sharedParticles, gopts));

	// Default to the first page and update the left bar visual highlight accordingly.
	rightContent->setCurrentIndex(0);
	leftButtons->press_button(0);
}
