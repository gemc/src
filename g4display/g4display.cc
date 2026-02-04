// g4display.cc
//
// Implementation of the G4Display main widget.
// Doxygen documentation for public behavior is authoritative in g4display.h (see rule 7).

// gemc
#include "g4display.h"
#include "g4display_options.h"       // provides G4DISPLAY_LOGGER constant and option definitions
#include "tabs/g4displayview.h"      // control tab
#include "tabs/g4displayutilities.h" // utilities tab

G4Display::G4Display(const std::shared_ptr<GOptions>& gopt, QWidget* parent)
	: GBase(gopt, G4DISPLAY_LOGGER),
	  QWidget(parent) {
	// QTabWidget hosts the visualization control panels.
	auto dialogTabs = new QTabWidget;

	// View tab: camera/light/slice/view flags.
	dialogTabs->addTab(new G4DisplayView(gopt, log, this), tr("View"));

	// Utilities tab: auxiliary tools (currently a placeholder).
	dialogTabs->addTab(new G4DisplayUtilities(gopt, log, this), tr("Utilities"));

	// Lay out the tab widget as the main content of this widget.
	auto* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(dialogTabs);
	setLayout(mainLayout);

	log->debug(NORMAL, SFUNCTION_NAME, "View and Utilities tabs added.");
}
