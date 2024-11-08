#ifndef  G4DIALOG_H
#define  G4DIALOG_H  1

// qt
#include <QtWidgets>

// gemc
#include "goptions.h"

class G4Dialog : public QTabWidget {

public:
    G4Dialog(GOptions *gopt, QWidget *parent = Q_NULLPTR);

};


#endif
