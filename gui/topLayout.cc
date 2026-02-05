// gui
#include "gui.h"


// qt
#include <QString>
#include <QCoreApplication>


void GemcGUI::createTopButtons(QHBoxLayout* topLayout) {
	// Build the top run-control row:
	// - number-of-events field
	// - run/cycle/stop/exit buttons
	// - current event counter label
	auto* nEventsLabel = new QLabel(tr("N. Events:"));

	auto* runButton = new QPushButton(tr("Run"));
	runButton->setToolTip("Run events");
	runButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

	// getting the number of events from eventDispenser, which
	// in turns gets it from options if no grun file is provided
	auto grunNumberOfEvents = std::to_string(eventDispenser->getTotalNumberOfEvents());
	nEvents                 = new QLineEdit(tr(grunNumberOfEvents.c_str()));
	nEvents->setMaximumWidth(50);

	auto* cycleButton = new QPushButton(tr("Cycle"));
	cycleButton->setToolTip("Run 1 event every 2 seconds");
	cycleButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));

	auto* stopButton = new QPushButton(tr("Stop"));
	stopButton->setToolTip("Stops running events");
	stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));

	auto* closeButton = new QPushButton(tr("Exit"));
	closeButton->setToolTip("Quit GEMC");
	closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));

	// The label stores the cumulative number of processed events (as displayed to the user).
	eventNumberLabel = new QLabel(tr("Event Number: 0"));

	topLayout->addWidget(nEventsLabel);
	topLayout->addWidget(nEvents);
	topLayout->addWidget(runButton);
	topLayout->addWidget(cycleButton);
	topLayout->addWidget(stopButton);
	topLayout->addStretch(1);
	topLayout->addWidget(eventNumberLabel);
	topLayout->addStretch(40);
	topLayout->addWidget(closeButton);

	// Wire UI events to slots:
	// - text edits update the backend event count
	// - buttons trigger run/cycle/stop/quit behavior
	connect(nEvents, &QLineEdit::textChanged, this, &GemcGUI::neventsChanged);
	connect(closeButton, &QPushButton::clicked, this, &GemcGUI::gquit);
	connect(runButton, &QPushButton::clicked, this, &GemcGUI::beamOn);
	connect(cycleButton, &QPushButton::clicked, this, &GemcGUI::cycleBeamOn);
	connect(stopButton, &QPushButton::clicked, this, &GemcGUI::stopCycleBeamOn);
}


void GemcGUI::gquit() {
	// Request application shutdown through Qt's application object.
	qApp->quit();
}


void GemcGUI::neventsChanged() {
	// Push the new event count into the backend so the next run uses the updated value.
	int newNevents = nEvents->text().toInt();
	eventDispenser->setNumberOfEvents(newNevents);
}

void GemcGUI::beamOn() {
	// Run a batch once, then update the GUI counter label.
	eventDispenser->processEvents();
	updateGui();
}


void GemcGUI::cycleBeamOn() {
	// Enable periodic processing (2 seconds interval) and process events for this cycle.
	gtimer->start(2000);
	eventDispenser->processEvents();
}


void GemcGUI::stopCycleBeamOn() { gtimer->stop(); }
