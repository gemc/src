#ifndef  G4DISPLAYUTILITIES_H
#define  G4DISPLAYUTILITIES_H 1

// qt
#include <QtWidgets>

// glibrary
#include "goptions.h"

// geant4 headers
#include "G4UImanager.hh"

class G4DisplayUtilities: public QWidget {

	// metaobject required for non-qt slots
	Q_OBJECT

public:
	G4DisplayUtilities(GOptions* gopt, QWidget* parent = nullptr);

	// no need to delete the pointers below, done by qt parenting
	~G4DisplayUtilities() {}

private:



};


#endif
