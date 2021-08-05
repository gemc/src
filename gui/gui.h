#ifndef GEMCGUI_H
#define GEMCGUI_H 1

// c++
#include <string>

// qt
#include <QtWidgets>

// gemc
#include "gStateMessage.h"

// glibrary
#include "gQtButtonsWidget.h"
#include "goptions.h"
#include "eventDispenser.h"

class GemcGUI : public QWidget, public GStateMessage
{
	// metaobject required for non-qt slots
	Q_OBJECT

public:
	GemcGUI(string qtResourceFile, GOptions* gopts, EventDispenser *ed, QWidget *parent = Q_NULLPTR) ;
	~GemcGUI();


private:
	GQTButtonsWidget *leftButtons;  // glibrary buttons
	QStackedWidget   *rightContent;
	QLineEdit        *nEvents;
	QLabel           *eventNumber;
	QTimer           *gtimer;       // for cycling events

	// gruns to read number of events
	EventDispenser *eventDispenser;
	
private:
	void createLeftButtons();
	void createRightContent(GOptions* gopts);
	void createTopButtons(QHBoxLayout *topLayout);
	void updateGui();

private slots:
	// definded in topLayout.cc
	// beamOn() causes workers to update the screen
	// from a sub-thread
	void beamOn();
	void cycleBeamOn();
	void stopCycleBeamOn();
	void gquit();
};

#endif
