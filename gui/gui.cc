// gui
#include "gui.h"
#include "gtree.h"
#include "g4SceneProperties.h"

// geant4
#include "G4UImanager.hh"



GemcGUI::GemcGUI(std::shared_ptr<GOptions>       gopts,
                 std::shared_ptr<EventDispenser> ed,
                 GDetectorConstruction*          dc,
                 QWidget*                        parent) :
	QWidget(parent),
	eventDispenser(ed),
	guiOptions(gopts),
	detectorConstruction(dc) {

	// Create the left navigation pane first; right content initialization uses it to sync selection state.
	createLeftButtons();           // instantiates leftButtons

	// The board is a long-lived widget attached to this GUI; it is used by other pages (e.g. dialog).
	auto* gboard = new GBoard(gopts, this);

	// Session object ties together options and the board; it is kept alive for the GUI lifetime.
	guiSession = std::make_unique<GUI_Session>(gopts, gboard);

	// Create the right stacked content pages (display, dialog, setup, tree) and synchronize default selection.
	createRightContent(gopts, dc, gboard); // instantiates rightContent: g4control, g4dialog, etc

	// Top row control buttons (run controls and counters).
	auto* topLayout = new QHBoxLayout;
	createTopButtons(topLayout);

	// Bottom row contains left navigation and right content; stretch factor favors the content area.
	auto* bottomLayout = new QHBoxLayout;
	// second argument is stretch factor. Right content can have 10 times more space.
	bottomLayout->addWidget(leftButtons, 1);
	bottomLayout->addWidget(rightContent, 10);

	// Main layout: top controls, bottom panes, and board.
	auto* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(bottomLayout);
	mainLayout->addWidget(gboard);

	setLayout(mainLayout);
	setWindowTitle(tr("GEMC: Geant4 Monte-Carlo"));
	setFixedWidth(1000);

	// Timer used for cycle mode; timeouts are connected to cycleBeamOn().
	gtimer = new QTimer(this);
	connect(gtimer, SIGNAL(timeout()), this, SLOT(cycleBeamOn()));

	// Page switching: left button selection drives the stacked widget page index.
	connect(leftButtons->buttonsWidget,
	        SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem*)),
	        this, SLOT(change_page(QListWidgetItem *, QListWidgetItem*)));
}


void GemcGUI::updateGui() {
	// Parse the current label text and increment by the amount run in the most recent batch.
	std::vector<std::string> sBefore = gutilities::getStringVectorFromString(eventNumberLabel->text().toStdString());

	int nThatWasRun = nEvents->text().toInt();
	int nBefore     = stoi(sBefore[2]);

	QString newNEvents("Event Number: ");
	newNEvents.append(std::to_string(nBefore + nThatWasRun).c_str());

	eventNumberLabel->setText(newNEvents);
}


void GemcGUI::refreshGeometryTree() {
	if (!rightContent || !detectorConstruction || !guiOptions || !geometryTree) { return; }

	const int treeIndex = rightContent->indexOf(geometryTree);
	if (treeIndex < 0) { return; }

	const int currentIndex = rightContent->currentIndex();
	auto* refreshedTree = new GTree(guiOptions, detectorConstruction->get_g4volumes_map());

	rightContent->removeWidget(geometryTree);
	geometryTree->deleteLater();
	geometryTree = refreshedTree;
	rightContent->insertWidget(treeIndex, geometryTree);
	rightContent->setCurrentIndex(currentIndex == treeIndex ? treeIndex : currentIndex);

	if (detectorConstruction->get_g4volumes_map().size() > 1) {
		G4SceneProperties g4SceneProperties(guiOptions);
		auto* g4uim = G4UImanager::GetUIpointer();
		if (!g4uim) { return; }

		auto commands = g4SceneProperties.scene_commands(guiOptions);
		commands.emplace_back("/run/initialize");
		commands.emplace_back("/vis/drawVolume");
		commands.emplace_back("/vis/scene/add/trajectories smooth");
		commands.emplace_back("/vis/modeling/trajectories/create/drawByCharge");
		commands.emplace_back("/vis/modeling/trajectories/drawByCharge-0/default/setDrawStepPts true");
		commands.emplace_back("/vis/modeling/trajectories/drawByCharge-0/default/setStepPtsSize 2");
		commands.emplace_back("/vis/scene/add/hits");
		commands.emplace_back("/vis/scene/endOfEventAction accumulate 10000");
		commands.emplace_back("/vis/viewer/set/background 0.05 0.05 0.26");
		commands.emplace_back("/vis/viewer/flush");

		for (const auto& command : commands) { g4uim->ApplyCommand(command); }
	}
}


GemcGUI::~GemcGUI() {
	// All widgets are owned via Qt parent/child relationships once placed into the layout hierarchy.
	// Rely on Qt to destroy children when GemcGUI is destroyed.
}


void GemcGUI::change_page(QListWidgetItem* current, QListWidgetItem* previous) {

	// Qt may emit selection changes with a null current item; fall back to the previous selection.
	if (!current)
		current = previous;

	// The left button widget provides the active page index; apply it to the stacked widget.
	int thisIndex = leftButtons->button_pressed();
	rightContent->setCurrentIndex(thisIndex);
}
