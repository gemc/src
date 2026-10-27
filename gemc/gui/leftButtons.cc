// gemc
#include "gui.h"

void GemcGUI::createLeftButtons() {
	// Build the icon list in the same order as the right-side stacked widget pages.
	std::vector<std::string> bicons;

	bicons.push_back(":/gui/images/buttons/display");
	bicons.push_back(":/gui/images/buttons/setup");
	bicons.push_back(":/gui/images/buttons/tree");
	bicons.push_back(":/gui/images/buttons/dialog");
	bicons.push_back(":/gui/images/buttons/generator");
	bicons.push_back(":/gui/images/buttons/analyzer");
	bicons.push_back(":/gui/images/buttons/physics");

	// The button widget provides both the list UI and the selected index used for page switching.
	leftButtons = new GQTButtonsWidget(96, 96, bicons);
	const QStringList names{tr("Display"), tr("Setup"), tr("Geometry"), tr("Commands"),
	                        tr("Generator"), tr("Analyzer"), tr("Physics")};
	for (int index = 0; index < names.size(); ++index) {
		leftButtons->buttonsWidget->item(index)->setToolTip(names[index]);
		leftButtons->buttonsWidget->item(index)->setData(Qt::AccessibleTextRole, names[index]);
	}
}
