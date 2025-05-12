// gemc
#include "g4display.h"           // Class definition
#include "g4display_options.h" // Provides G4DISPLAY_LOGGER constant and option definitions
#include "tabs/g4displayview.h"      // View control tab
#include "tabs/g4displayutilities.h" // Utilities tab

// G4Display Constructor
// Initializes the QTabWidget base class and the unique_ptr for the logger.
// Creates and adds the specific control tabs.
G4Display::G4Display(GOptions* gopt, QWidget* parent) :
		QWidget(parent), log(std::make_shared<GLogger>(gopt, G4DISPLAY_LOGGER, "G4Display"))
{
    // Log the construction event using the newly created logger.
    // Use level 0 info for general construction messages.
	log->debug(CONSTRUCTOR, "G4Display");

	auto dialogTabs = new QTabWidget;

    // Create and add the 'View' tab.
    // Pass the GOptions, the logger instance (retrieved via log.get()), and this widget as the parent.
	dialogTabs->addTab(new G4DisplayView(gopt, log, this), tr("View"));

    // Create and add the 'Utilities' tab.
    // Pass the GOptions, the logger instance, and this widget as the parent.
	dialogTabs->addTab(new G4DisplayUtilities(gopt, log, this), tr("Utilities"));

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(dialogTabs);
	setLayout(mainLayout);

    // Optional: Log successful initialization
    log->debug(NORMAL, "View and Utilities tabs added.");
}

// G4Display Destructor
// The unique_ptr 'log' will automatically delete the GLogger instance here.
G4Display::~G4Display() {
	log->debug(DESTRUCTOR, "G4Display");
}