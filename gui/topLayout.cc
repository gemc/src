// gui
#include "gui.h"

// geant4 headers
#include "G4UImanager.hh"

// qt
#include <QString>

// c++
using namespace std;



void GemcGUI::createTopButtons(QHBoxLayout *topLayout) {
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

    connect(nEvents, &QLineEdit::textChanged, this, &GemcGUI::neventsChanged);
    connect(closeButton, &QPushButton::clicked, this, &GemcGUI::gquit);
    connect(runButton, &QPushButton::clicked, this, &GemcGUI::beamOn);
    connect(cycleButton, &QPushButton::clicked, this, &GemcGUI::cycleBeamOn);
    connect(stopButton, &QPushButton::clicked, this, &GemcGUI::stopCycleBeamOn);
}


void GemcGUI::gquit() {
    qApp->quit();
}


void GemcGUI::neventsChanged() {
    int newNevents = nEvents->text().toInt();
    eventDispenser->setNumberOfEvents(newNevents);
}

void GemcGUI::beamOn() {
    eventDispenser->processEvents();
    updateGui();
}


void GemcGUI::cycleBeamOn() {
    gtimer->start(2000);
    eventDispenser->processEvents();
}


void GemcGUI::stopCycleBeamOn() {
    gtimer->stop();
}
