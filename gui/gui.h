#pragma once

// qt
#include <QStackedWidget>   // rightContent
#include <QLineEdit>        // nEvents
#include <QLabel>           // eventNumberLabel
#include <QTimer>           // gtimer
#include <QHBoxLayout>      // createTopButtons

// gemc
#include "gQtButtonsWidget.h"
#include "eventDispenser.h"
#include "gdetectorConstruction.h"

class GemcGUI : public QWidget {

	// metaobject required for non-qt slots
	Q_OBJECT

public:
	GemcGUI(std::shared_ptr<GOptions>       gopts,
	        std::shared_ptr<EventDispenser> ed,
	        GDetectorConstruction*          dc,
	        QWidget*                        parent = nullptr);

	~GemcGUI() override;

private:
	GQTButtonsWidget* leftButtons;  // left bar buttons
	QStackedWidget*   rightContent; // pages controlled by left bar buttons
	QLineEdit*        nEvents;
	QLabel*           eventNumberLabel;
	QTimer*           gtimer; // for cycling events

	// EventDispenser to run beamOn
	std::shared_ptr<EventDispenser> eventDispenser;

private:
	void createLeftButtons();

	void createRightContent(std::shared_ptr<GOptions> gopts,
	                        GDetectorConstruction*    dc);

	void createTopButtons(QHBoxLayout* topLayout);

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

	void change_page(QListWidgetItem* current, QListWidgetItem* previous);
};
