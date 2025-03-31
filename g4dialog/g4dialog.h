#ifndef  G4DIALOG_H
#define  G4DIALOG_H  1

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
		delete log;
	}

private:
	GLogger * const log;
	GBoard* gboard;
	GUI_Session *gui_session;
};


#endif
