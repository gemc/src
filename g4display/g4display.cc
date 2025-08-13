// gemc
#include "g4display.h"
#include "g4display_options.h"       // provides G4DISPLAY_LOGGER constant and option definitions
#include "tabs/g4displayview.h"      // control tab
#include "tabs/g4displayutilities.h" // utilities tab

// G4Display Constructor
// Initializes the QTabWidget base class and the unique_ptr for the logger.
// Creates and adds the specific control tabs.
G4Display::G4Display(const std::shared_ptr<GOptions>& gopt, QWidget* parent) :
	GBase(gopt, G4DISPLAY_LOGGER),
	QWidget(parent) {
	auto dialogTabs = new QTabWidget;

	// Create and add the 'View' tab.
	// Pass the GOptions, the logger instance (retrieved via log.get()), and this widget as the parent.
	dialogTabs->addTab(new G4DisplayView(gopt, log, this), tr("View"));

	// Create and add the 'Utilities' tab.
	// Pass the GOptions, the logger instance, and this widget as the parent.
	dialogTabs->addTab(new G4DisplayUtilities(gopt, log, this), tr("Utilities"));

	auto* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(dialogTabs);
	setLayout(mainLayout);

	log->debug(NORMAL, SFUNCTION_NAME, "View and Utilities tabs added.");
}
