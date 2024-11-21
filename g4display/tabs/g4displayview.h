#ifndef  G4DISPLAYVIEW_H
#define  G4DISPLAYVIEW_H 1

// qt
#include <QtWidgets>

// glibrary
#include "goptions.h"
#include "gQtButtonsWidget.h"

// geant4 headers
#include "G4UImanager.hh"

class G4DisplayView : public QWidget {
Q_OBJECT // Required for non-qt signals/slots

public:
    G4DisplayView(GOptions *gopts, QWidget *parent = Q_NULLPTR);

    // no need to delete the pointers below, done by qt parenting
    ~G4DisplayView() {}

private:

    GQTToggleButtonWidget *buttons_set1;  // buttons
    QSlider *cameraTheta;
    QSlider *cameraPhi;
    QSlider *lightTheta;
    QSlider *lightPhi;

    // slices
    QLineEdit *sliceXEdit, *sliceYEdit, *sliceZEdit;
    QCheckBox *sliceXActi, *sliceYActi, *sliceZActi;
    QCheckBox *sliceXInve, *sliceYInve, *sliceZInve;

    // intersection or union
    QRadioButton *sliceSectn;
    QRadioButton *sliceUnion;


private slots:
    void changeCameraDirection();

    void changeLightDirection();

    void slice();

    void clearSlices();

    void apply_buttons_set1(int index);
};


#endif
