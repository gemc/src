// gemc
#include "gui.h"

// geant4 headers
#include "G4UImanager.hh"

// qt
#include <QString>

// c++
using namespace std;

// glibrary
//#include "gutilities.h"
//using namespace gutilities;

//#include <future>

void GemcGUI::createTopButtons(QHBoxLayout *topLayout)
{
	QLabel *nEventsLabel = new QLabel(tr("N. Events:"));

	QPushButton *runButton = new QPushButton(tr("Run"));
	runButton->setToolTip("Run events");
	runButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));

	// getting number of events from eventDispenser, which
	// in turns gets it from options if no grun file is provided
	auto grunNumberOfEvents = to_string(eventDispenser->getTotalNumberOfEvents());
	nEvents = new QLineEdit(tr(grunNumberOfEvents.c_str()));
	nEvents->setMaximumWidth(50);

	QPushButton *cycleButton = new QPushButton(tr("Cycle"));
	cycleButton->setToolTip("Run 1 event every 2 seconds");
	cycleButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));

	QPushButton *stopButton = new QPushButton(tr("Stop"));
	stopButton->setToolTip("Stops running events");
	stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));

	QPushButton *closeButton = new QPushButton(tr("Exit"));
	closeButton->setToolTip("Quit GEMC");
	closeButton->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));

	eventNumberLabel  = new QLabel(tr("Event Number: 0"));

	topLayout->addWidget(nEventsLabel);
	topLayout->addWidget(nEvents);
	topLayout->addWidget(runButton);
	topLayout->addWidget(cycleButton);
	topLayout->addWidget(stopButton);
	topLayout->addStretch(1);
	topLayout->addWidget(eventNumberLabel);
	topLayout->addStretch(40);
	topLayout->addWidget(closeButton);

	connect(closeButton, &QPushButton::clicked, this, &GemcGUI::gquit);
	connect(runButton,   &QPushButton::clicked, this, &GemcGUI::beamOn);
	connect(cycleButton, &QPushButton::clicked, this, &GemcGUI::cycleBeamOn);
	connect(stopButton,  &QPushButton::clicked, this, &GemcGUI::stopCycleBeamOn);
}


void GemcGUI::gquit()
{
	qApp->quit();
}


void GemcGUI::beamOn()
{
	// PRAGMA TODO: GRUNS must be able to redistribute events
	// int nToRun  = nEvents->text().toInt();

	G4UImanager *g4uim = G4UImanager::GetUIpointer();
	
	// interestingly enough this the accumulate directive will prevent this:
	// coreAnimation: warning, deleted thread with uncommitted CATransaction
	// PRAGMA TODO: make this a button
	g4uim->ApplyCommand("/vis/scene/add/trajectories rich smooth");
	g4uim->ApplyCommand("/vis/scene/endOfEventAction accumulate -1");
	g4uim->ApplyCommand("/vis/scene/add/hits");
//	g4uim->ApplyCommand("/vis/scene/endOfRunAction accumulate -1");
//	g4uim->ApplyCommand("/event/keepCurrentEvent");
//	g4uim->ApplyCommand("/vis/ogl/flushAt endOfEvent");
//	g4uim->ApplyCommand("/vis/ogl/flushAt never");
	
	g4uim->ApplyCommand("/gun/particle proton");
	g4uim->ApplyCommand("/gun/energy 2 GeV");
	g4uim->ApplyCommand("/gun/direction 0 1 0");

//	g4uim->ApplyCommand("/event/keepCurrentEvent");
//	g4uim->ApplyCommand("/vis/disable");

//	g4uim->ApplyCommand( "/run/beamOn 100");
//	g4uim->ApplyCommand( "/run/beamOn " + to_string(nToRun));

//	g4uim->ApplyCommand("/vis/enable");
//	g4uim->ApplyCommand("/vis/viewer/flush");
//	g4uim->ApplyCommand("/vis/reviewKeptEvents");
	//g4uim->ApplyCommand("/vis/viewer/flush");
	
	eventDispenser->processEvents();
	
	updateGui();

}


void GemcGUI::cycleBeamOn()
{
	
}


void GemcGUI::stopCycleBeamOn()
{
	
}
