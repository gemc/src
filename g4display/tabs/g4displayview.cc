#include "g4displayview.h"
#include "../g4display_options.h"
#include "gutilities.h"

using namespace g4display;

#include <iostream>
#include <string>

using namespace std;
using namespace gutilities;

/**
 * @brief Constructs the G4DisplayView widget.
 *
 * Retrieves initial camera settings from GOptions, sets up the UI elements (toggle buttons,
 * camera and light direction sliders, slice controls, and field precision input), arranges them
 * with appropriate layouts, and connects UI signals to the corresponding slots.
 *
 * @param gopts Pointer to the GOptions object containing initial configuration.
 * @param logger Pointer to the shared GLogger instance for logging.
 * @param parent Optional pointer to the parent QWidget.
 */
G4DisplayView::G4DisplayView(GOptions* gopts, GLogger* const logger, QWidget* parent) : QWidget(parent), log(logger) {
	log->debug(CONSTRUCTOR, "G4DisplayView");

	G4Camera jcamera = getG4Camera(gopts);

	double thetaValue = getG4Number(jcamera.theta);
	double phiValue = getG4Number(jcamera.phi);

	vector<string> toggle_button_titles;
	toggle_button_titles.emplace_back("Hidden\nLines");
	toggle_button_titles.emplace_back("Anti\nAliasing");
	toggle_button_titles.emplace_back("Auxiliary\nEdges");
	toggle_button_titles.emplace_back("Field\nLines");


	buttons_set1 = new GQTToggleButtonWidget(80, 80, 20, toggle_button_titles, false, this);
	connect(buttons_set1, &GQTToggleButtonWidget::buttonPressedIndexChanged, this, &G4DisplayView::apply_buttons_set1);

	// Camera sliders
	cameraTheta = new QSlider(Qt::Horizontal);
	cameraTheta->setRange(0, 180);
	cameraTheta->setSingleStep(1);
	cameraTheta->setValue(thetaValue);
	auto cameraThetaLabel = new QLabel(tr("θ"));

	auto cameraThetaLayout = new QHBoxLayout;
	cameraThetaLayout->addWidget(cameraThetaLabel);
	cameraThetaLayout->addWidget(cameraTheta);

	cameraPhi = new QSlider(Qt::Horizontal);
	cameraPhi->setRange(0, 360);
	cameraPhi->setSingleStep(1);
	cameraPhi->setValue(phiValue);
	auto cameraPhiLabel = new QLabel(tr("ɸ"));

	auto cameraPhiLayout = new QHBoxLayout;
	cameraPhiLayout->addWidget(cameraPhiLabel);
	cameraPhiLayout->addWidget(cameraPhi);

	QVBoxLayout* cameraDirectionLayout = new QVBoxLayout;
	cameraDirectionLayout->addLayout(cameraThetaLayout);
	cameraDirectionLayout->addSpacing(12);
	cameraDirectionLayout->addLayout(cameraPhiLayout);

	QGroupBox* cameraAnglesGroup = new QGroupBox(tr("Camera Direction"));
	cameraAnglesGroup->setLayout(cameraDirectionLayout);

	connect(cameraTheta, &QSlider::valueChanged, this, &G4DisplayView::changeCameraDirection);
	connect(cameraPhi, &QSlider::valueChanged, this, &G4DisplayView::changeCameraDirection);

	// Light Direction
	lightTheta = new QSlider(Qt::Horizontal);
	lightTheta->setRange(0, 180);
	lightTheta->setSingleStep(1);
	lightTheta->setValue(thetaValue);
	auto lightThetaLabel = new QLabel(tr("θ"));

	auto lightThetaLayout = new QHBoxLayout;
	lightThetaLayout->addWidget(lightThetaLabel);
	lightThetaLayout->addWidget(lightTheta);

	lightPhi = new QSlider(Qt::Horizontal);
	lightPhi->setRange(0, 360);
	lightPhi->setSingleStep(1);
	lightPhi->setValue(phiValue);
	auto lightPhiLabel = new QLabel(tr("ɸ"));

	auto lightPhiLayout = new QHBoxLayout;
	lightPhiLayout->addWidget(lightPhiLabel);
	lightPhiLayout->addWidget(lightPhi);

	auto lightDirectionLayout = new QVBoxLayout;
	lightDirectionLayout->addLayout(lightThetaLayout);
	lightDirectionLayout->addSpacing(12);
	lightDirectionLayout->addLayout(lightPhiLayout);

	QGroupBox* lightAnglesGroup = new QGroupBox(tr("Light Direction"));
	lightAnglesGroup->setLayout(lightDirectionLayout);

	connect(lightTheta, &QSlider::valueChanged, this, &G4DisplayView::changeLightDirection);
	connect(lightPhi, &QSlider::valueChanged, this, &G4DisplayView::changeLightDirection);

	// x slice
	sliceXEdit = new QLineEdit(tr("0"));
	sliceXEdit->setMaximumWidth(100);
	sliceXActi = new QCheckBox(tr("&On"));
	sliceXActi->setChecked(false);
	sliceXInve = new QCheckBox(tr("&Flip"));
	sliceXInve->setChecked(false);
	auto sliceXLayout = new QHBoxLayout;
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
	auto sliceYLayout = new QHBoxLayout;
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
	auto sliceZLayout = new QHBoxLayout;
	sliceZLayout->addWidget(new QLabel(tr("Z: ")));
	sliceZLayout->addWidget(sliceZEdit);
	sliceZLayout->addStretch(1);
	sliceZLayout->addWidget(sliceZActi);
	sliceZLayout->addWidget(sliceZInve);
	sliceZLayout->addStretch(1);

	// clear sice button
	QPushButton* clearSliceButton = new QPushButton(tr("Clear Slices"));
	clearSliceButton->setToolTip("Clear Slice Planes");
	clearSliceButton->setIcon(QIcon::fromTheme("edit-clear"));
	clearSliceButton->setIconSize(QSize(16, 16)); // Adjust as necessary

	connect(clearSliceButton, &QPushButton::clicked, this, &G4DisplayView::clearSlices);

	// slice style: Intersection or Union
	QGroupBox* sliceChoiceBox = new QGroupBox(tr("Slices Style"));
	sliceSectn = new QRadioButton(tr("&Intersection"), sliceChoiceBox);
	sliceUnion = new QRadioButton(tr("&Union"), sliceChoiceBox);
	sliceSectn->setChecked(true);

	connect(sliceSectn, &QRadioButton::toggled, this, &G4DisplayView::slice);
	connect(sliceUnion, &QRadioButton::toggled, this, &G4DisplayView::slice);

	auto sliceChoiceLayout = new QHBoxLayout;
	sliceChoiceLayout->addWidget(sliceSectn);
	sliceChoiceLayout->addWidget(sliceUnion);
	sliceChoiceBox->setLayout(sliceChoiceLayout);


	// slices layout
	auto sliceLayout = new QVBoxLayout;
	sliceLayout->addLayout(sliceXLayout);
	sliceLayout->addLayout(sliceYLayout);
	sliceLayout->addLayout(sliceZLayout);
	sliceLayout->addWidget(sliceChoiceBox);
	sliceLayout->addWidget(clearSliceButton);


	// connecting widgets to slice
	connect(sliceXEdit, &QLineEdit::returnPressed, this, &G4DisplayView::slice);
	connect(sliceYEdit, &QLineEdit::returnPressed, this, &G4DisplayView::slice);
	connect(sliceZEdit, &QLineEdit::returnPressed, this, &G4DisplayView::slice);

	connect(sliceXActi, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
	connect(sliceYActi, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
	connect(sliceZActi, &QCheckBox::stateChanged, this, &G4DisplayView::slice);

	connect(sliceXInve, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
	connect(sliceYInve, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
	connect(sliceZInve, &QCheckBox::stateChanged, this, &G4DisplayView::slice);


	QGroupBox* fieldPrecisionBox = new QGroupBox(tr("Number of Field Points"));
	field_npoints = new QLineEdit(QString::number(field_NPOINTS), this);
	field_npoints->setMaximumWidth(40);

	QFont font = field_npoints->font();
	font.setPointSize(24);
	field_npoints->setFont(font);
	connect(field_npoints, &QLineEdit::returnPressed, this, &G4DisplayView::field_precision_changed);

	// buttons + field line number
	auto fieldPointsHBox = new QHBoxLayout;
	fieldPointsHBox->addWidget(field_npoints);
	fieldPrecisionBox->setLayout(fieldPointsHBox);

	auto buttons_field_HBox = new QHBoxLayout;
	buttons_field_HBox->addWidget(buttons_set1);
	buttons_field_HBox->addWidget(fieldPrecisionBox);
	fieldPrecisionBox->setMaximumHeight(3 * buttons_set1->height());
	fieldPrecisionBox->setMaximumWidth(140);


	// All layouts together
	auto mainLayout = new QVBoxLayout;
	mainLayout->addLayout(buttons_field_HBox);
	mainLayout->addWidget(cameraAnglesGroup);
	mainLayout->addWidget(lightAnglesGroup);
	mainLayout->addLayout(sliceLayout);
	setLayout(mainLayout);
}

/**
 * @brief Slot triggered when camera direction sliders change.
 *
 * Constructs and sends the Geant4 command to update the viewpoint based on the
 * current theta and phi values from the camera sliders.
 */
void G4DisplayView::changeCameraDirection() {
	// Construct the command using the current slider values.
	string command = "/vis/viewer/set/viewpointThetaPhi " +
					 to_string(cameraTheta->value()) + " " +
					 to_string(cameraPhi->value());
	// Send the command to the Geant4 UImanager.
	G4UImanager::GetUIpointer()->ApplyCommand(command);
}

/**
 * @brief Slot triggered when light direction sliders change.
 *
 * Constructs and sends the Geant4 command to update the light direction based on
 * the current theta and phi values from the light sliders.
 */
void G4DisplayView::changeLightDirection() {
	string command = "/vis/viewer/set/lightsThetaPhi " +
					 to_string(lightTheta->value()) + " " +
					 to_string(lightPhi->value());
	G4UImanager::GetUIpointer()->ApplyCommand(command);
}


/**
 * @brief Slot that applies slice settings to the Geant4 visualization.
 *
 * Clears existing cutaway planes, sets the cutaway mode based on the radio buttons,
 * and then adds new cutaway planes for each active slice control (X, Y, Z) with the
 * appropriate normal vector (inverted if needed).
 */
void G4DisplayView::slice() {
	G4UImanager* g4uim = G4UImanager::GetUIpointer();
	if (g4uim == nullptr) { return; }

	// can't have a mix of wireframe / solid when doing a slice.
	// forcing all to be solid
	//	if(!solidVis) {
	//		g4uim->ApplyCommand("/vis/geometry/set/forceSolid all -1 1");
	//	}

	g4uim->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	if (sliceSectn->isChecked()) { g4uim->ApplyCommand("/vis/viewer/set/cutawayMode intersection"); }
	else if (sliceUnion->isChecked()) { g4uim->ApplyCommand("/vis/viewer/set/cutawayMode union"); }

	g4uim->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	if (sliceXActi->isChecked()) {
		string command = "/vis/viewer/addCutawayPlane " + sliceXEdit->text().toStdString() + " 0  0 mm " +
			to_string(sliceXInve->isChecked() ? -1 : 1) + " 0 0 ";
		cout << "X " << command << endl;
		g4uim->ApplyCommand(command);
	}

	if (sliceYActi->isChecked()) {
		string command = "/vis/viewer/addCutawayPlane 0 " + sliceYEdit->text().toStdString() + " 0 mm 0 " +
			to_string(sliceYInve->isChecked() ? -1 : 1) + " 0 ";
		cout << "Y " << command << endl;
		g4uim->ApplyCommand(command);
	}

	if (sliceZActi->isChecked()) {
		string command = "/vis/viewer/addCutawayPlane 0 0 " + sliceZEdit->text().toStdString() + " mm 0 0 " +
			to_string(sliceZInve->isChecked() ? -1 : 1);
		cout << "Z " << command << endl;
		g4uim->ApplyCommand(command);
	}

	//solidVis = true;
}


/**
 * @brief Slot triggered by the "Clear Slices" button.
 *
 * Clears all cutaway planes and resets the X slice activation checkbox.
 */
void G4DisplayView::clearSlices() {
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/clearCutawayPlanes");
	sliceXActi->setChecked(false);
}

/**
 * @brief Slot that applies toggle button settings.
 *
 * Responds to changes in the toggle button group (buttons_set1) by sending the
 * appropriate Geant4 commands based on the toggled button's index and state.
 *
 * @param index The index of the toggled button:
 *        0: Hidden Lines, 1: Anti-Aliasing, 2: Auxiliary Edges, 3: Field Lines.
 */
void G4DisplayView::apply_buttons_set1(int index) {
	G4UImanager* g4uim = G4UImanager::GetUIpointer();
	if (g4uim == nullptr) { return; }

	bool button_state = buttons_set1->lastButtonState();

	if (index == 0) {
		string command = string("/vis/viewer/set/hiddenEdge") + (button_state ? " 1" : " 0");
		g4uim->ApplyCommand(command);
		g4uim->ApplyCommand("/vis/viewer/flush");
	}
	else if (index == 1) {
		if (button_state == 0) {
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_POLYGON_SMOOTH);
		}
		else {
			glEnable(GL_LINE_SMOOTH);
			glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
			glEnable(GL_POLYGON_SMOOTH);
			glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		}
	}
	else if (index == 2) {
		string command = string("/vis/viewer/set/auxiliaryEdge") + (button_state ? " 1" : " 0");
		g4uim->ApplyCommand(command);
		command = string("/vis/viewer/set/hiddenEdge") + (button_state ? " 1" : " 0");
		g4uim->ApplyCommand(command);
		if (buttons_set1->buttonStatus(0) != button_state) { buttons_set1->toggleButton(0); }
	}
	else if (index == 3) {
		if (button_state == 0) {
			string command = string("vis/scene/activateModel Field 0");
			g4uim->ApplyCommand(command);
			g4uim->ApplyCommand("/vis/scene/removeModel Field");
		}
		else {
			string npoints = to_string(field_NPOINTS);
			string command = string("/vis/scene/add/magneticField ") + npoints;
			g4uim->ApplyCommand(command);
		}
	}
}

/**
 * @brief Slot triggered by changes in the field precision QLineEdit.
 *
 * Reads the new field point value from the line edit, updates the internal variable,
 * and if the field lines are currently active, reconfigures the magnetic field visualization.
 */
void G4DisplayView::field_precision_changed() {
	G4UImanager* g4uim = G4UImanager::GetUIpointer();
	if (g4uim == nullptr) { return; }
	field_NPOINTS = field_npoints->text().toInt();
	if (buttons_set1->buttonStatus(3) == 1) {
		string command = string("vis/scene/activateModel Field 0");
		g4uim->ApplyCommand(command);
		g4uim->ApplyCommand("/vis/scene/removeModel Field");

		string npoints = to_string(field_NPOINTS);
		command = string("/vis/scene/add/magneticField ") + npoints;
		G4UImanager::GetUIpointer()->ApplyCommand(command);
	}
}
