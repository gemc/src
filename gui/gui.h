#pragma once

// c++
#include <string>

// qt
#include <QtWidgets>

// gemc
#include "gStateMessage.h"
#include "gQtButtonsWidget.h"
#include "goptions.h"
#include "eventDispenser.h"
#include "gdetectorConstruction.h"

class GemcGUI : public QWidget, public GStateMessage {
    // metaobject required for non-qt slots
    Q_OBJECT

public:
    GemcGUI(string qtResourceFile, GOptions *gopts, EventDispenser *ed, GDetectorConstruction *dc, QWidget *parent = nullptr);

    ~GemcGUI();


private:
    GQTButtonsWidget *leftButtons;  // buttons
    QStackedWidget *rightContent;
    QLineEdit *nEvents;
    QLabel *eventNumberLabel;
    QTimer *gtimer;       // for cycling events

    // EventDispenser to run beamOn
    EventDispenser *eventDispenser;


private:
    void createLeftButtons();

    void createRightContent(GOptions *gopts, GDetectorConstruction *dc);

    void createTopButtons(QHBoxLayout *topLayout);

    void updateGui();

private slots:
    // defined in topLayout.cc
    // beamOn() causes workers to update the screen
    // from a sub-thread
    void neventsChanged();

    void beamOn();

    void cycleBeamOn();

    void stopCycleBeamOn();

    void gquit();

    void change_page(QListWidgetItem *current, QListWidgetItem *previous);
};


