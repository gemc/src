// g4display
#include "../g4displayOptions.h"
#include "g4displaycamera.h"
using namespace g4display;

// glibrary
#include "gutilities.h"
using namespace gutilities;

// c++
#include <string>
using namespace std;

G4DisplayCamera::G4DisplayCamera(GOptions* gopts, QWidget* parent) : QWidget(parent)
{
	// projecting option onto JView
	JCamera jcamera = getJCamera(gopts);

	double thetaValue = getG4Number(jcamera.theta);
	double phiValue   = getG4Number(jcamera.phi);


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

	// all layouts together
	// --------------------
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addWidget(cameraAnglesGroup);
	mainLayout->addWidget(lightAnglesGroup);
	setLayout(mainLayout);
}

void G4DisplayCamera::changeCameraDirection()
{
	string command = "/vis/viewer/set/viewpointThetaPhi " + to_string(cameraTheta->value()) + " " + to_string(cameraPhi->value());
	G4UImanager::GetUIpointer()->ApplyCommand(command);
}

void G4DisplayCamera::changeLightDirection()
{
	string command = "/vis/viewer/set/lightsThetaPhi " + to_string(lightTheta->value()) + " " + to_string(lightPhi->value());
	G4UImanager::GetUIpointer()->ApplyCommand(command);
}
