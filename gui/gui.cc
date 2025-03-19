// gui
#include "gui.h"

// gemc
#include "gutilities.h"

GemcGUI::GemcGUI([[maybe_unused]] string qtResourceFile, GOptions *gopts, EventDispenser *ed, GDetectorConstruction *dc,  QWidget *parent) :
        QWidget(parent),
        GStateMessage(gopts, "GemcGUI ", "verbosity"),  // GStateMessage derived
        eventDispenser(ed) {

    createLeftButtons();            // instantiates leftButtons
    createRightContent(gopts, dc);  // instantiates rightContent: g4control

    // top rows button
    QHBoxLayout *topLayout = new QHBoxLayout;
    createTopButtons(topLayout);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    // second argument is stretch factor. Right content can have 10 times more space.
    bottomLayout->addWidget(leftButtons, 1);
    bottomLayout->addWidget(rightContent, 10);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);
    setWindowTitle(tr("GEMC: Geant4 Monte-Carlo"));
    setFixedWidth(800);

    // setting timer
    gtimer = new QTimer(this);
    connect(gtimer, SIGNAL(timeout()), this, SLOT(cycleBeamOn()));

    // connect GQTButtonsWidget signal button_pressed to slot change_page()
    connect(leftButtons->buttonsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem*)),
            this, SLOT(change_page(QListWidgetItem *, QListWidgetItem*)));
}


void GemcGUI::updateGui() {
    vector <string> sBefore = gutilities::getStringVectorFromString(eventNumberLabel->text().toStdString());

    int nBefore = stoi(sBefore[2]);
    int nThatWasRun = nEvents->text().toInt();

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

void GemcGUI::change_page(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    int thisIndex = leftButtons->button_pressed();
    rightContent->setCurrentIndex(thisIndex);
}
