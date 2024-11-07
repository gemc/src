#ifndef  G4DISPLAY_H
#define  G4DISPLAY_H  1


// qt
#include <QtWidgets>

// glibrary
#include "goptions.h"

class G4Display : public QTabWidget {

public:
    G4Display(GOptions *gopt, QWidget *parent = Q_NULLPTR);

};


#endif
