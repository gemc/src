#ifndef G4DISPLAYVIEW_H
#define G4DISPLAYVIEW_H 1

// Qt
#include <QtWidgets>

// gemc
#include "goptions.h"
#include "gQtButtonsWidget.h"

// Geant4
#include "G4UImanager.hh"

class G4DisplayView : public QWidget {
	Q_OBJECT

public:
	G4DisplayView(GOptions *gopts, QWidget *parent = nullptr);

	~G4DisplayView() override {}

private:
	QLineEdit *field_npoints;

	GQTToggleButtonWidget *buttons_set1;
	QSlider *cameraTheta, *cameraPhi;
	QSlider *lightTheta, *lightPhi;

	QLineEdit *sliceXEdit, *sliceYEdit, *sliceZEdit;
	QCheckBox *sliceXActi, *sliceYActi, *sliceZActi;
	QCheckBox *sliceXInve, *sliceYInve, *sliceZInve;

	QRadioButton *sliceSectn, *sliceUnion;

	int field_NPOINTS = 5;

private slots:
	void changeCameraDirection();

	void changeLightDirection();

	void slice();

	void clearSlices();

	void apply_buttons_set1(int index);

	void field_precision_changed();
};

#endif
