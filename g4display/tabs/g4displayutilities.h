#ifndef  G4DISPLAYUTILITIES_H
#define  G4DISPLAYUTILITIES_H 1

// qt
#include <QtWidgets>

// gemc
#include "glogger.h"

// geant4 headers
#include "G4UImanager.hh"

class G4DisplayUtilities : public QWidget {
	// metaobject required for non-qt slots
	Q_OBJECT

public:
	G4DisplayUtilities(GOptions* gopt, GLogger* const logger,  QWidget* parent = nullptr);

	// no need to delete the pointers below, done by qt parenting
	~G4DisplayUtilities() { log->debug(DESTRUCTOR, "G4DisplayUtilities"); }

private:
	GLogger* const log;
};


#endif
