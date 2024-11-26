// g4display
#include "../g4displayOptions.h"
#include "g4displayview.h"
using namespace g4display;

// glibrary
#include "gutilities.h"
using namespace gutilities;

// c++
#include <string>
using namespace std;

G4DisplayView::G4DisplayView(GOptions* gopts, QWidget* parent) : QWidget(parent)
{
	// projecting option onto G4View
	G4Camera jcamera = getG4Camera(gopts);

	double thetaValue = getG4Number(jcamera.theta);
	double phiValue   = getG4Number(jcamera.phi);

	vector <string> toggle_button_titles;
	toggle_button_titles.push_back("Hidden\nLines");
	toggle_button_titles.push_back("Anti\nAliasing");
	toggle_button_titles.push_back("Auxiliary\nEdges");
	toggle_button_titles.push_back("Field\nLines");

	buttons_set1 = new GQTToggleButtonWidget(80, 80, 20, toggle_button_titles, false, this);
    connect(buttons_set1, SIGNAL(buttonPressedIndexChanged(int)), this, SLOT(apply_buttons_set1(int)));


	// Camera Direction Group
	// ----------------------------

	// THETA
	cameraTheta = new QSlider(Qt::Horizontal);
	cameraTheta->setRange(0, 180);
	cameraTheta->setSingleStep(1);
	cameraTheta->setValue(thetaValue);
	QLabel *cameraThetaLabel = new QLabel(tr("θ"));

	QHBoxLayout *cameraThetaLayout = new QHBoxLayout;
	cameraThetaLayout->addWidget(cameraThetaLabel);
	cameraThetaLayout->addWidget(cameraTheta);

	// PHI
	cameraPhi = new  QSlider(Qt::Horizontal);
	cameraPhi->setRange(0, 360);
	cameraPhi->setSingleStep(1);
	cameraPhi->setValue(phiValue);
	QLabel *cameraPhiLabel = new QLabel(tr("ɸ"));

	QHBoxLayout *cameraPhiLayout = new QHBoxLayout;
	cameraPhiLayout->addWidget(cameraPhiLabel);
	cameraPhiLayout->addWidget(cameraPhi);

	// Combine THETA and PHI
	QVBoxLayout *cameraDirectionLayout = new QVBoxLayout;
	cameraDirectionLayout->addLayout(cameraThetaLayout);
	cameraDirectionLayout->addSpacing(12);
	cameraDirectionLayout->addLayout(cameraPhiLayout);

	QGroupBox *cameraAnglesGroup = new QGroupBox(tr("Camera Direction"));
	cameraAnglesGroup->setLayout(cameraDirectionLayout);

	connect ( cameraTheta, SIGNAL( valueChanged(int) ), this, SLOT( changeCameraDirection() ) );
	connect ( cameraPhi,   SIGNAL( valueChanged(int) ), this, SLOT( changeCameraDirection() ) );


	// Light Direction and background colors
	// ----------------------

	// THETA
	lightTheta = new QSlider(Qt::Horizontal);
	lightTheta->setRange(0, 180);
	lightTheta->setSingleStep(1);
	lightTheta->setValue(thetaValue);
	QLabel *lightThetaLabel = new QLabel(tr("θ"));

	QHBoxLayout *lightThetaLayout = new QHBoxLayout;
	lightThetaLayout->addWidget(lightThetaLabel);
	lightThetaLayout->addWidget(lightTheta);

	// PHI
	lightPhi = new  QSlider(Qt::Horizontal);
	lightPhi->setRange(0, 360);
	lightPhi->setSingleStep(1);
	lightPhi->setValue(phiValue);
	QLabel *lightPhiLabel = new QLabel(tr("ɸ"));

	QHBoxLayout *lightPhiLayout = new QHBoxLayout;
	lightPhiLayout->addWidget(lightPhiLabel);
	lightPhiLayout->addWidget(lightPhi);

	// Combine THETA and PHI
	QVBoxLayout *lightDirectionLayout = new QVBoxLayout;
	lightDirectionLayout->addLayout(lightThetaLayout);
	lightDirectionLayout->addSpacing(12);
	lightDirectionLayout->addLayout(lightPhiLayout);

	QGroupBox *lightAnglesGroup = new QGroupBox(tr("Light Direction"));
	lightAnglesGroup->setLayout(lightDirectionLayout);

	connect ( lightTheta, SIGNAL( valueChanged(int) ), this, SLOT( changeLightDirection() ) );
	connect ( lightPhi,   SIGNAL( valueChanged(int) ), this, SLOT( changeLightDirection() ) );


	// x slice
	sliceXEdit = new QLineEdit(tr("0"));
	sliceXEdit->setMaximumWidth(100);

	sliceXActi = new QCheckBox(tr("&On"));
	sliceXActi->setChecked(false);

	sliceXInve = new QCheckBox(tr("&Flip"));
	sliceXInve->setChecked(false);

	QHBoxLayout *sliceXLayout = new QHBoxLayout;
	sliceXLayout->addWidget(new QLabel(tr("X: ")));
	sliceXLayout->addWidget(sliceXEdit);
	sliceXLayout->addStretch(1);
	sliceXLayout->addWidget(sliceXActi);
	sliceXLayout->addWidget(sliceXInve);
	sliceXLayout->addStretch(1);


	// y slice
	sliceYEdit = new QLineEdit(tr("0"));
	sliceYEdit->setMaximumWidth(100);

	sliceYActi = new QCheckBox(tr("&On"));
	sliceYActi->setChecked(false);

	sliceYInve = new QCheckBox(tr("&Flip"));
	sliceYInve->setChecked(false);

	QHBoxLayout *sliceYLayout = new QHBoxLayout;
	sliceYLayout->addWidget(new QLabel(tr("Y: ")));
	sliceYLayout->addWidget(sliceYEdit);
	sliceYLayout->addStretch(1);
	sliceYLayout->addWidget(sliceYActi);
	sliceYLayout->addWidget(sliceYInve);
	sliceYLayout->addStretch(1);



	// z slice
	sliceZEdit = new QLineEdit(tr("0"));
	sliceZEdit->setMaximumWidth(100);

	sliceZActi = new QCheckBox(tr("&On"));
	sliceZActi->setChecked(false);

	sliceZInve = new QCheckBox(tr("&Flip"));
	sliceZInve->setChecked(false);

	QHBoxLayout *sliceZLayout = new QHBoxLayout;
	sliceZLayout->addWidget(new QLabel(tr("Z: ")));
	sliceZLayout->addWidget(sliceZEdit);
	sliceZLayout->addStretch(1);
	sliceZLayout->addWidget(sliceZActi);
	sliceZLayout->addWidget(sliceZInve);
	sliceZLayout->addStretch(1);


	// clear sice button
	QPushButton *clearSliceButton = new QPushButton(tr("Clear Slices"));
	clearSliceButton->setToolTip("Clear Slice Planes");
	clearSliceButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
	connect ( clearSliceButton , SIGNAL(clicked()), this, SLOT( clearSlices() ) );


	// slice style: Intersection or Union
	QGroupBox *sliceChoiceBox = new QGroupBox(tr("Slices Style"));
	sliceSectn = new QRadioButton(tr("&Intersection"), sliceChoiceBox);
	sliceUnion = new QRadioButton(tr("&Union"),        sliceChoiceBox);
	sliceSectn->setChecked(true);

	connect ( sliceSectn , SIGNAL(clicked()), this, SLOT( slice() ) );
	connect ( sliceUnion , SIGNAL(clicked()), this, SLOT( slice() ) );

	QHBoxLayout *sliceChoiceLayout = new QHBoxLayout;
	sliceChoiceLayout->addWidget(sliceSectn);
	sliceChoiceLayout->addWidget(sliceUnion);
	sliceChoiceBox->setLayout(sliceChoiceLayout);


	// slices layout
	QVBoxLayout *sliceLayout = new QVBoxLayout;
	sliceLayout->addLayout(sliceXLayout);
	sliceLayout->addLayout(sliceYLayout);
	sliceLayout->addLayout(sliceZLayout);
	sliceLayout->addWidget(sliceChoiceBox);
	sliceLayout->addWidget(clearSliceButton);


	// connecting widgets to slice
	connect ( sliceXEdit , SIGNAL( returnPressed() ), this, SLOT( slice() ) );
	connect ( sliceYEdit , SIGNAL( returnPressed() ), this, SLOT( slice() ) );
	connect ( sliceZEdit , SIGNAL( returnPressed() ), this, SLOT( slice() ) );

	connect ( sliceXActi , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );
	connect ( sliceYActi , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );
	connect ( sliceZActi , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );

	connect ( sliceXInve , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );
	connect ( sliceYInve , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );
	connect ( sliceZInve , SIGNAL( stateChanged(int) ), this, SLOT( slice() ) );


	// all layouts together
	// --------------------
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(buttons_set1);
	mainLayout->addWidget(cameraAnglesGroup);
	mainLayout->addWidget(lightAnglesGroup);
	mainLayout->addLayout(sliceLayout);
	setLayout(mainLayout);
}

void G4DisplayView::changeCameraDirection()
{
	string command = "/vis/viewer/set/viewpointThetaPhi " + to_string(cameraTheta->value()) + " " + to_string(cameraPhi->value());
	G4UImanager::GetUIpointer()->ApplyCommand(command);
}

void G4DisplayView::changeLightDirection()
{
	string command = "/vis/viewer/set/lightsThetaPhi " + to_string(lightTheta->value()) + " " + to_string(lightPhi->value());
	G4UImanager::GetUIpointer()->ApplyCommand(command);
}



void G4DisplayView::slice()
{
	G4UImanager *g4uim = G4UImanager::GetUIpointer();
	if (g4uim == nullptr) {
		return;
	}

	// can't have a mix of wireframe / solid when doing a slice.
	// forcing all to be solid
//	if(!solidVis) {
//		g4uim->ApplyCommand("/vis/geometry/set/forceSolid all -1 1");
//	}

	g4uim->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	if(sliceSectn->isChecked()) {
		g4uim->ApplyCommand("/vis/viewer/set/cutawayMode intersection");
	} else if(sliceUnion->isChecked()) {
		g4uim->ApplyCommand("/vis/viewer/set/cutawayMode union");
	}

	g4uim->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	if(sliceXActi->isChecked() ) {
		string command = "/vis/viewer/addCutawayPlane "     + sliceXEdit->text().toStdString() + " 0  0 mm " + to_string(sliceXInve->isChecked() ? -1 : 1) + " 0 0 ";
		cout << "X " << command << endl;
		g4uim->ApplyCommand(command);
	}

	if(sliceYActi->isChecked() ) {
		string command = "/vis/viewer/addCutawayPlane 0 "   + sliceYEdit->text().toStdString() + " 0 mm 0 " + to_string(sliceYInve->isChecked() ? -1 : 1) + " 0 ";
		cout << "Y " << command << endl;
		g4uim->ApplyCommand(command);
	}

	if(sliceZActi->isChecked() ) {
		string command = "/vis/viewer/addCutawayPlane 0 0 " + sliceZEdit->text().toStdString() + " mm 0 0 " + to_string(sliceZInve->isChecked() ? -1 : 1);
		cout << "Z " << command << endl;
		g4uim->ApplyCommand(command);
	}

	//solidVis = true;

}

void G4DisplayView::clearSlices()
{
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	sliceXActi->setChecked(false);
	sliceYActi->setChecked(false);
	sliceZActi->setChecked(false);
}

void G4DisplayView::apply_buttons_set1(int index) {

    int button_index = buttons_set1->buttonPressed();
    bool button_state = buttons_set1->lastButtonState();

	G4UImanager *g4uim = G4UImanager::GetUIpointer();
	if (g4uim == nullptr) {
		return;
	}

    if (button_index == 0) {
        string command = string("/vis/viewer/set/hiddenEdge") + (button_state ? " 1" : " 0");
        g4uim->ApplyCommand(command);
    } else if (button_index == 1) {
		if (button_state == 0) {
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_POLYGON_SMOOTH);
		} else {
			glEnable(GL_LINE_SMOOTH);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			glEnable(GL_POLYGON_SMOOTH);
			glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		}
		g4uim->ApplyCommand("/vis/viewer/flush");
	} else if (button_index == 2) {
		string command = string("/vis/viewer/set/auxiliaryEdge") + (button_state ? " 1" : " 0");
		g4uim->ApplyCommand(command);
		command = string("/vis/viewer/set/hiddenEdge") + (button_state ? " 1" : " 0");
		g4uim->ApplyCommand(command);
		if (buttons_set1->buttonStatus(0) != button_state) {
			buttons_set1->toggleButton(0);
		}
	} else if (button_index == 3) {
		if (button_state == 0) {
			string command = string("vis/scene/activateModel Field false");
			g4uim->ApplyCommand(command);
			command = string("/vis/viewer/flush");
			g4uim->ApplyCommand(command);
		} else {
			string npoints = "100";
			string command = string("/vis/scene/add/magneticField ") + npoints;
			g4uim->ApplyCommand(command);
			command = string("/vis/viewer/flush");
			g4uim->ApplyCommand(command);
		}
	}
}
