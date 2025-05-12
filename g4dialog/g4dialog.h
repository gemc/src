#pragma once

// g4dialog
#include "tabs/gboard.h"
#include "gui_session.h"

// qt
#include <QtWidgets>

// gemc
#include "glogger.h"

class G4Dialog : public QWidget {

public:
	explicit G4Dialog(GOptions *gopt, QWidget *parent = nullptr);

    ~G4Dialog() override {
		log->debug(DESTRUCTOR, "G4Dialog");
		delete gboard;
	}

private:
	std::shared_ptr<GLogger> log;
	GBoard* gboard;
	GUI_Session *gui_session;
};

