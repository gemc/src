// gui
#include "gui.h"

// gboard
#include "gui_session.h"

GemcGUI::GemcGUI(std::shared_ptr<GOptions>       gopts,
                 std::shared_ptr<EventDispenser> ed,
                 GDetectorConstruction*          dc,
                 QWidget*                        parent) :
	QWidget(parent),
	eventDispenser(ed) {
	createLeftButtons();           // instantiates leftButtons
	auto* gboard = new GBoard(gopts, this);
	auto gui_session = std::make_unique<GUI_Session>(gopts, gboard);

	createRightContent(gopts, dc, gboard); // instantiates rightContent: g4control, g4dialog, etc

	// top rows button
	auto* topLayout = new QHBoxLayout;
	createTopButtons(topLayout);

	auto* bottomLayout = new QHBoxLayout;
	// second argument is stretch factor. Right content can have 10 times more space.
	bottomLayout->addWidget(leftButtons, 1);
	bottomLayout->addWidget(rightContent, 10);

	auto* mainLayout = new QVBoxLayout;
	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(bottomLayout);
	mainLayout->addWidget(gboard);

	setLayout(mainLayout);
	setWindowTitle(tr("GEMC: Geant4 Monte-Carlo"));
	setFixedWidth(1000);

	// setting timer
	gtimer = new QTimer(this);
	connect(gtimer, SIGNAL(timeout()), this, SLOT(cycleBeamOn()));

	// connect GQTButtonsWidget signal button_pressed to slot change_page()
	connect(leftButtons->buttonsWidget,
	        SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem*)),
	        this, SLOT(change_page(QListWidgetItem *, QListWidgetItem*)));
}


void GemcGUI::updateGui() {
	std::vector<std::string> sBefore = gutilities::getStringVectorFromString(eventNumberLabel->text().toStdString());

	int nThatWasRun = nEvents->text().toInt();
	int nBefore     = stoi(sBefore[2]);

	QString newNEvents("Event Number: ");
	newNEvents.append(std::to_string(nBefore + nThatWasRun).c_str());

	eventNumberLabel->setText(newNEvents);
}


GemcGUI::~GemcGUI() {
	delete leftButtons;
	delete rightContent;
	delete nEvents;
	delete eventNumberLabel;
}

void GemcGUI::change_page(QListWidgetItem* current, QListWidgetItem* previous) {

	if (!current)
		current = previous;

	int thisIndex = leftButtons->button_pressed();
	rightContent->setCurrentIndex(thisIndex);
}
