#ifndef  G4CONTROLS_H
#define  G4CONTROLS_H  1


// qt
#include <QtWidgets>

// glibrary
#include "goptions.h"

class  G4Controls : public QTabWidget
{

public:
	G4Controls(GOptions* gopt, QWidget* parent = Q_NULLPTR);

};


#endif
