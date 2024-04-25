#ifndef  G4DISPLAYSLICE_H
#define  G4DISPLAYSLICE_H 1

// qt
#include <QtWidgets>

// glibrary
#include "goptions.h"

// geant4 headers
#include "G4UImanager.hh"

class G4DisplaySlice: public QWidget {

	// metaobject required for non-qt slots
	Q_OBJECT

public:
	G4DisplaySlice(GOptions* gopt, QWidget* parent = Q_NULLPTR);

	// no need to delete the pointers below, done by qt parenting
	~G4DisplaySlice() {}

private:

	// slices
	QLineEdit *sliceXEdit, *sliceYEdit, *sliceZEdit;
	QCheckBox *sliceXActi, *sliceYActi, *sliceZActi;
	QCheckBox *sliceXInve, *sliceYInve, *sliceZInve;

	// intersection or union
	QRadioButton *sliceSectn;
	QRadioButton *sliceUnion;

	private slots:
	void slice();
	void clearSlices();

};


#endif
