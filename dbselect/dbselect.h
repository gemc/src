#ifndef DBSELECT_H
#define DBSELECT_H 1

// Qt
#include <QtWidgets>

// gemc
#include "goptions.h"
#include "gQtButtonsWidget.h"

// Geant4
#include "G4UImanager.hh"

class DBSelectView : public QWidget {
	Q_OBJECT

public:
	DBSelectView(GOptions *gopts, QWidget *parent = nullptr);

	~DBSelectView() override {}

private:


private slots:

};

#endif
