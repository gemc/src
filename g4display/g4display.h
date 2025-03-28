#ifndef  G4DISPLAY_H
#define  G4DISPLAY_H  1

// gemc
#include "glogger.h"

// qt
#include <QtWidgets>


class G4Display : public QTabWidget {

public:
    G4Display(GOptions *gopt, QWidget *parent = nullptr);

private:
	GLogger * const log;
};


#endif
