#include "g4displayview.h"
#include "g4display_options.h"
#include "gutilities.h"

using namespace g4display;

// c++
#include <iostream>
#include <string>
using namespace std;
using namespace gutilities;

// geant4
#include "G4UImanager.hh" // Geant4 UI manager access

// Implementation note:
// Detailed Doxygen documentation for public behavior and slots is maintained in g4displayview.h (see rule 7).

G4DisplayView::G4DisplayView(const std::shared_ptr<GOptions>& gopts,
                             std::shared_ptr<GLogger>         logger,
                             QWidget*                         parent)
	: QWidget(parent), log(logger) {
	log->debug(CONSTRUCTOR, "G4DisplayView");

	// LCD font used to display theta/phi degrees next to sliders.
	QFont flcd;
	flcd.setFamilies({"Helvetica"}); // Qt6: prefer setFamilies over family string ctor
	flcd.setPointSize(32);
	flcd.setBold(true);

	// Initial camera configuration comes from options; values are converted to degrees for UI.
	G4Camera jcamera    = getG4Camera(gopts);
	double   thetaValue = getG4Number(jcamera.theta);
	double   phiValue   = getG4Number(jcamera.phi);

	// Toggle buttons for common viewer/scene flags.
	vector<string> toggle_button_titles;
	toggle_button_titles.emplace_back("Hidden\nLines");
	toggle_button_titles.emplace_back("Anti\nAliasing");
	toggle_button_titles.emplace_back("Auxiliary\nEdges");
	toggle_button_titles.emplace_back("Field\nLines");
	toggle_button_titles.emplace_back("Axes");
	toggle_button_titles.emplace_back("Scale");

	buttons_set1 = new GQTToggleButtonWidget(80, 80, 20, toggle_button_titles, false, this);
	connect(buttons_set1, &GQTToggleButtonWidget::buttonPressedIndexChanged, this, &G4DisplayView::apply_buttons_set1);

	// Preset angle sets used by camera and light dropdowns.
	QStringList theta_angle_Set;
	for (int t = 0; t <= 180; t += 30) { theta_angle_Set << QString::number(t); }
	QStringList phi_angle_Set;
	for (int t = 0; t <= 360; t += 30) { phi_angle_Set << QString::number(t); }

	// -------------------------
	// Camera direction controls
	// -------------------------

	cameraTheta = new QSlider(Qt::Horizontal);
	cameraTheta->setRange(0, 180);
	cameraTheta->setSingleStep(1);
	cameraTheta->setValue(thetaValue);
	cameraTheta->setTracking(true); // updates while dragging

	auto cameraThetaLabel = new QLabel(tr("θ"));

	QLCDNumber* theta_LCD = new QLCDNumber(this);
	theta_LCD->setFont(flcd);
	theta_LCD->setMaximumSize(QSize(45, 45));
	theta_LCD->setSegmentStyle(QLCDNumber::Flat);

	thetaDropdown = new QComboBox(this);
	thetaDropdown->addItems(theta_angle_Set);
	thetaDropdown->setMaximumSize(QSize(100, 45));

	auto cameraThetaLayout = new QHBoxLayout;
	cameraThetaLayout->addWidget(cameraThetaLabel);
	cameraThetaLayout->addWidget(cameraTheta);
	cameraThetaLayout->addWidget(theta_LCD);
	cameraThetaLayout->addWidget(thetaDropdown);

	cameraPhi = new QSlider(Qt::Horizontal);
	cameraPhi->setRange(0, 360);
	cameraPhi->setSingleStep(1);
	cameraPhi->setValue(phiValue);
	cameraPhi->setTracking(true); // updates while dragging

	auto cameraPhiLabel = new QLabel(tr("ɸ"));

	QLCDNumber* phi_LCD = new QLCDNumber(this);
	phi_LCD->setFont(flcd);
	phi_LCD->setMaximumSize(QSize(45, 45));
	phi_LCD->setSegmentStyle(QLCDNumber::Flat);

	phiDropdown = new QComboBox(this);
	phiDropdown->addItems(phi_angle_Set);
	phiDropdown->setMaximumSize(QSize(100, 45));

	auto cameraPhiLayout = new QHBoxLayout;
	cameraPhiLayout->addWidget(cameraPhiLabel);
	cameraPhiLayout->addWidget(cameraPhi);
	cameraPhiLayout->addWidget(phi_LCD);
	cameraPhiLayout->addWidget(phiDropdown);

	QVBoxLayout* cameraDirectionLayout = new QVBoxLayout;
	cameraDirectionLayout->addLayout(cameraThetaLayout);
	cameraDirectionLayout->addSpacing(12);
	cameraDirectionLayout->addLayout(cameraPhiLayout);

	QGroupBox* cameraAnglesGroup = new QGroupBox(tr("Camera Direction"));
	cameraAnglesGroup->setLayout(cameraDirectionLayout);

	// Slider -> Geant4 command, and slider -> LCD.
	connect(cameraTheta, &QSlider::valueChanged, this, &G4DisplayView::changeCameraDirection);
	connect(cameraTheta, &QSlider::valueChanged, theta_LCD, qOverload<int>(&QLCDNumber::display));

	// Dropdown -> Geant4 command, and dropdown -> slider sync.
	connect(thetaDropdown, &QComboBox::currentTextChanged,
	        this, [this](const QString&) { setCameraDirection(0); });

	connect(cameraPhi, &QSlider::valueChanged, this, &G4DisplayView::changeCameraDirection);
	connect(cameraPhi, &QSlider::valueChanged, phi_LCD, qOverload<int>(&QLCDNumber::display));

	connect(phiDropdown, &QComboBox::currentTextChanged,
	        this, [this](const QString&) { setCameraDirection(1); });

	// ---------------------
	// View properties group
	// ---------------------

	// Projection: orthogonal or perspective (with typical angles).
	QLabel* projLabel   = new QLabel(tr("Projection:"));
	perspectiveDropdown = new QComboBox;
	perspectiveDropdown->addItem(tr("Orthogonal"));
	perspectiveDropdown->addItem(tr("Perspective 30"));
	perspectiveDropdown->addItem(tr("Perspective 45"));
	perspectiveDropdown->addItem(tr("Perspective 60"));

	// Sides per circle: maps to /vis/viewer/set/lineSegmentsPerCircle.
	QLabel* sides_per_circlesLabel = new QLabel(tr("Sides per circle:"));
	precisionDropdown              = new QComboBox;
	precisionDropdown->addItem(tr("50"));
	precisionDropdown->addItem(tr("100"));
	precisionDropdown->addItem(tr("200"));
	precisionDropdown->addItem(tr("300"));
	precisionDropdown->setCurrentIndex(0);

	connect(perspectiveDropdown, &QComboBox::currentTextChanged, this, &G4DisplayView::set_projection);
	connect(precisionDropdown, &QComboBox::currentTextChanged, this, &G4DisplayView::set_precision);

	QVBoxLayout* resolutionAndPerspectiveLayout = new QVBoxLayout;
	resolutionAndPerspectiveLayout->addWidget(projLabel);
	resolutionAndPerspectiveLayout->addWidget(perspectiveDropdown);
	resolutionAndPerspectiveLayout->addSpacing(12);
	resolutionAndPerspectiveLayout->addWidget(sides_per_circlesLabel);
	resolutionAndPerspectiveLayout->addWidget(precisionDropdown);

	QGroupBox* propertyGroup = new QGroupBox(tr("View Properties"));
	propertyGroup->setLayout(resolutionAndPerspectiveLayout);

	QHBoxLayout* cameraAndPerspective = new QHBoxLayout;
	cameraAndPerspective->addWidget(cameraAnglesGroup);
	cameraAndPerspective->addSpacing(12);
	cameraAndPerspective->addWidget(propertyGroup);

	// -------------------------
	// Light direction controls
	// -------------------------

	lightTheta = new QSlider(Qt::Horizontal);
	lightTheta->setRange(0, 180);
	lightTheta->setSingleStep(1);
	lightTheta->setValue(thetaValue);
	lightTheta->setTracking(true);

	auto lightThetaLabel = new QLabel(tr("θ"));

	QLCDNumber* ltheta_LCD = new QLCDNumber(this);
	ltheta_LCD->setFont(flcd);
	ltheta_LCD->setMaximumSize(QSize(45, 45));
	ltheta_LCD->setSegmentStyle(QLCDNumber::Flat);

	lthetaDropdown = new QComboBox(this);
	lthetaDropdown->addItems(theta_angle_Set);
	lthetaDropdown->setMaximumSize(QSize(100, 45));

	auto lightThetaLayout = new QHBoxLayout;
	lightThetaLayout->addWidget(lightThetaLabel);
	lightThetaLayout->addWidget(lightTheta);
	lightThetaLayout->addWidget(ltheta_LCD);
	lightThetaLayout->addWidget(lthetaDropdown);

	lightPhi = new QSlider(Qt::Horizontal);
	lightPhi->setRange(0, 360);
	lightPhi->setSingleStep(1);
	lightPhi->setValue(phiValue);
	lightPhi->setTracking(true);

	auto lightPhiLabel = new QLabel(tr("ɸ"));

	QLCDNumber* lphi_LCD = new QLCDNumber(this);
	lphi_LCD->setFont(flcd);
	lphi_LCD->setMaximumSize(QSize(45, 45));
	lphi_LCD->setSegmentStyle(QLCDNumber::Flat);

	lphiDropdown = new QComboBox(this);
	lphiDropdown->addItems(phi_angle_Set);
	lphiDropdown->setMaximumSize(QSize(100, 45));

	auto lightPhiLayout = new QHBoxLayout;
	lightPhiLayout->addWidget(lightPhiLabel);
	lightPhiLayout->addWidget(lightPhi);
	lightPhiLayout->addWidget(lphi_LCD);
	lightPhiLayout->addWidget(lphiDropdown);

	auto lightDirectionLayout = new QVBoxLayout;
	lightDirectionLayout->addLayout(lightThetaLayout);
	lightDirectionLayout->addSpacing(12);
	lightDirectionLayout->addLayout(lightPhiLayout);

	QGroupBox* lightAnglesGroup = new QGroupBox(tr("Light Direction"));
	lightAnglesGroup->setLayout(lightDirectionLayout);

	connect(lightTheta, &QSlider::valueChanged, this, &G4DisplayView::changeLightDirection);
	connect(lightTheta, &QSlider::valueChanged, ltheta_LCD, qOverload<int>(&QLCDNumber::display));
	connect(lthetaDropdown, &QComboBox::currentTextChanged,
	        this, [this](const QString&) { setLightDirection(0); });

	connect(lightPhi, &QSlider::valueChanged, this, &G4DisplayView::changeLightDirection);
	connect(lightPhi, &QSlider::valueChanged, lphi_LCD, qOverload<int>(&QLCDNumber::display));
	connect(lphiDropdown, &QComboBox::currentTextChanged,
	        this, [this](const QString&) { setLightDirection(1); });

	// ----------------------
	// Scene properties group
	// ----------------------

	QLabel* cullingLabel = new QLabel(tr("Culling:"));
	cullingDropdown      = new QComboBox;
	cullingDropdown->addItem(tr("Reset"));
	cullingDropdown->addItem(tr("Covered Daughters"));
	cullingDropdown->addItem(tr("Density: 1 mg/cm3"));
	cullingDropdown->addItem(tr("Density: 10 mg/cm3"));
	cullingDropdown->addItem(tr("Density: 100 mg/cm3"));
	cullingDropdown->addItem(tr("Density: 1 g/cm3"));
	cullingDropdown->addItem(tr("Density: 10 g/cm3"));

	QLabel* backgroundColorLabel = new QLabel(tr("Background Color:"));
	backgroundColorDropdown      = new QComboBox;
	backgroundColorDropdown->addItem(tr("lightslategray"));
	backgroundColorDropdown->addItem(tr("ghostwhite"));
	backgroundColorDropdown->addItem(tr("black"));
	backgroundColorDropdown->addItem(tr("navy"));
	backgroundColorDropdown->addItem(tr("whitesmoke"));
	backgroundColorDropdown->addItem(tr("lightskyblue"));
	backgroundColorDropdown->addItem(tr("deepskyblue"));
	backgroundColorDropdown->addItem(tr("lightsteelblue"));
	backgroundColorDropdown->addItem(tr("blueviolet"));
	backgroundColorDropdown->addItem(tr("turquoise"));
	backgroundColorDropdown->addItem(tr("mediumaquamarine"));
	backgroundColorDropdown->addItem(tr("springgreen"));
	backgroundColorDropdown->addItem(tr("lawngreen"));
	backgroundColorDropdown->addItem(tr("yellowgreen"));
	backgroundColorDropdown->addItem(tr("lemonchiffon"));
	backgroundColorDropdown->addItem(tr("antiquewhite"));
	backgroundColorDropdown->addItem(tr("wheat"));
	backgroundColorDropdown->addItem(tr("sienna"));
	backgroundColorDropdown->addItem(tr("snow"));
	backgroundColorDropdown->addItem(tr("floralwhite"));
	backgroundColorDropdown->addItem(tr("lightsalmon"));
	backgroundColorDropdown->addItem(tr("orchid"));
	backgroundColorDropdown->addItem(tr("plum"));
	backgroundColorDropdown->setCurrentIndex(0);

	connect(cullingDropdown, &QComboBox::currentTextChanged, this, &G4DisplayView::set_culling);
	connect(backgroundColorDropdown, &QComboBox::currentTextChanged, this, &G4DisplayView::set_background);

	QVBoxLayout* sceneLayout = new QVBoxLayout;
	sceneLayout->addWidget(cullingLabel);
	sceneLayout->addWidget(cullingDropdown);
	sceneLayout->addSpacing(12);
	sceneLayout->addWidget(backgroundColorLabel);
	sceneLayout->addWidget(backgroundColorDropdown);

	QGroupBox* spropertyGroup = new QGroupBox(tr("Scene Properties"));
	spropertyGroup->setLayout(sceneLayout);

	QHBoxLayout* lightAndProperties = new QHBoxLayout;
	lightAndProperties->addWidget(lightAnglesGroup);
	lightAndProperties->addSpacing(12);
	lightAndProperties->addWidget(spropertyGroup);

	// -------------------------
	// Slice (cutaway) controls
	// -------------------------

	// X slice controls.
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

	// Y slice controls.
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

	// Z slice controls.
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

	// Clear slice planes button.
	QPushButton* clearSliceButton = new QPushButton(tr("Clear Slices"));
	clearSliceButton->setToolTip("Clear Slice Planes");
	clearSliceButton->setIcon(QIcon::fromTheme("edit-clear"));
	clearSliceButton->setIconSize(QSize(16, 16));
	connect(clearSliceButton, &QPushButton::clicked, this, &G4DisplayView::clearSlices);

	// Slice composition mode: intersection vs union.
	QGroupBox* sliceChoiceBox = new QGroupBox(tr("Slices Style"));
	sliceSectn                = new QRadioButton(tr("&Intersection"), sliceChoiceBox);
	sliceUnion                = new QRadioButton(tr("&Union"), sliceChoiceBox);
	sliceSectn->setChecked(true);

	connect(sliceSectn, &QRadioButton::toggled, this, &G4DisplayView::slice);
	connect(sliceUnion, &QRadioButton::toggled, this, &G4DisplayView::slice);

	auto sliceChoiceLayout = new QHBoxLayout;
	sliceChoiceLayout->addWidget(sliceSectn);
	sliceChoiceLayout->addWidget(sliceUnion);
	sliceChoiceBox->setLayout(sliceChoiceLayout);

	// Slices layout.
	auto sliceLayout = new QVBoxLayout;
	sliceLayout->addLayout(sliceXLayout);
	sliceLayout->addLayout(sliceYLayout);
	sliceLayout->addLayout(sliceZLayout);
	sliceLayout->addWidget(sliceChoiceBox);
	sliceLayout->addWidget(clearSliceButton);

	// Connect slice UI signals to slice recomputation.
	connect(sliceXEdit, &QLineEdit::returnPressed, this, &G4DisplayView::slice);
	connect(sliceYEdit, &QLineEdit::returnPressed, this, &G4DisplayView::slice);
	connect(sliceZEdit, &QLineEdit::returnPressed, this, &G4DisplayView::slice);

#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0) // Qt ≤ 6.6
	connect(sliceXActi, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
	connect(sliceYActi, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
	connect(sliceZActi, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
	connect(sliceXInve, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
	connect(sliceYInve, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
	connect(sliceZInve, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
#else // Qt ≥ 6.7
	connect(sliceXActi, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
	connect(sliceYActi, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
	connect(sliceZActi, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
	connect(sliceXInve, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
	connect(sliceYInve, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
	connect(sliceZInve, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
#endif

	// ------------------------------
	// Field line precision controls
	// ------------------------------

	QGroupBox* fieldPrecisionBox = new QGroupBox(tr("Number of Field Points"));
	field_npoints                = new QLineEdit(QString::number(field_NPOINTS), this);
	field_npoints->setMaximumWidth(40);

	QFont font = field_npoints->font();
	font.setPointSize(24);
	field_npoints->setFont(font);

	connect(field_npoints, &QLineEdit::returnPressed, this, &G4DisplayView::field_precision_changed);

	// Buttons + field point count UI combined.
	auto fieldPointsHBox = new QHBoxLayout;
	fieldPointsHBox->addWidget(field_npoints);
	fieldPrecisionBox->setLayout(fieldPointsHBox);

	auto buttons_field_HBox = new QHBoxLayout;
	buttons_field_HBox->addWidget(buttons_set1);
	buttons_field_HBox->addWidget(fieldPrecisionBox);
	fieldPrecisionBox->setMaximumHeight(3 * buttons_set1->height());
	fieldPrecisionBox->setMaximumWidth(140);

	// -------------------------
	// Assemble final tab layout
	// -------------------------

	auto mainLayout = new QVBoxLayout;
	mainLayout->addLayout(buttons_field_HBox);
	mainLayout->addLayout(cameraAndPerspective);
	mainLayout->addLayout(lightAndProperties);
	mainLayout->addLayout(sliceLayout);
	setLayout(mainLayout);
}

void G4DisplayView::changeCameraDirection() {
	// Construct the Geant4 command using the current slider values and send it to the UI manager.
	string command = "/vis/viewer/set/viewpointThetaPhi " +
		to_string(cameraTheta->value()) + " " +
		to_string(cameraPhi->value());
	G4UImanager::GetUIpointer()->ApplyCommand(command);
}

void G4DisplayView::setCameraDirection(int which) {
	// Dropdown values are interpreted as degrees; command is issued first, then the slider is synced.
	string thetaValue = thetaDropdown->currentText().toStdString();
	string phiValue   = phiDropdown->currentText().toStdString();

	string command = "/vis/viewer/set/viewpointThetaPhi " + thetaValue + " " + phiValue;
	G4UImanager::GetUIpointer()->ApplyCommand(command);

	int thetaDeg = thetaDropdown->currentText().toInt();
	int phiDeg   = phiDropdown->currentText().toInt();

	if (cameraTheta && which == 0) cameraTheta->setValue(thetaDeg);
	if (cameraPhi && which == 1) cameraPhi->setValue(phiDeg);
}

void G4DisplayView::set_projection() {
	// Dropdown selections map to Geant4 projection parameters: orthogonal (o) or perspective (p, angle).
	string value = perspectiveDropdown->currentText().toStdString();

	string g4perspective = "o";
	string g4perpvalue   = "0";
	if (value.find("Perspective") != string::npos) {
		g4perspective = "p";
		// Second token after space is the angle.
		g4perpvalue = value.substr(value.find(" ") + 1);
	}

	string command = "/vis/viewer/set/projection " + g4perspective + " " + g4perpvalue;
	G4UImanager::GetUIpointer()->ApplyCommand(command);
}

void G4DisplayView::set_precision() {
	// Viewer circle segmentation impacts curved primitive smoothness.
	string value = precisionDropdown->currentText().toStdString();

	string command = "/vis/viewer/set/lineSegmentsPerCircle " + value;
	G4UImanager::GetUIpointer()->ApplyCommand(command);
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/flush");
}

void G4DisplayView::set_culling() {
	// Culling configuration uses Geant4 viewer culling commands.
	string value = cullingDropdown->currentText().toStdString();
	int    index = cullingDropdown->currentIndex() - 2;

	if (value.find("Reset") != string::npos) {
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/culling global true");
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/culling density false");
	}
	else if (value.find("Daughters") != string::npos) {
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/culling coveredDaughters true");
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/culling density false");
	}
	else {
		// Density-based thresholds (unit assumed by Geant4 command expectations).
		vector<double> density = {0.001, 0.01, 0.1, 1.0, 10.0, 100.0, 1000.0};
		string         command = "/vis/viewer/set/culling density true " + to_string(density[index]);
		G4UImanager::GetUIpointer()->ApplyCommand(command);
		G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/flush");
		log->info(0, command);
	}
}

void G4DisplayView::set_background() {
	// Translate CSS-like names to normalized RGB values expected by Geant4.
	string value = backgroundColorDropdown->currentText().toStdString();

	string g4value = "0 0 0";
	if (value.find("black") != string::npos) {
		g4value = "0.0 0.0 0.0";
	}
	else if (value.find("navy") != string::npos) {
		g4value = "0.0 0.0 0.50196";
	}
	else if (value.find("lightslategray") != string::npos) {
		g4value = "0.46667 0.53333 0.60000";
	}
	else if (value.find("whitesmoke") != string::npos) {
		g4value = "0.96078 0.96078 0.96078";
	}
	else if (value.find("ghostwhite") != string::npos) {
		g4value = "0.97255 0.97255 1.00000";
	}
	else if (value.find("lightskyblue") != string::npos) {
		g4value = "0.52941 0.80784 0.98039";
	}
	else if (value.find("deepskyblue") != string::npos) {
		g4value = "0.00000 0.74902 1.00000";
	}
	else if (value.find("lightsteelblue") != string::npos) {
		g4value = "0.69020 0.76863 0.87059";
	}
	else if (value.find("blueviolet") != string::npos) {
		g4value = "0.54118 0.16863 0.88627";
	}
	else if (value.find("turquoise") != string::npos) {
		g4value = "0.25098 0.87843 0.81569";
	}
	else if (value.find("mediumaquamarine") != string::npos) {
		g4value = "0.40000 0.80392 0.66667";
	}
	else if (value.find("springgreen") != string::npos) {
		g4value = "0.00000 1.00000 0.49804";
	}
	else if (value.find("lawngreen") != string::npos) {
		g4value = "0.48627 0.98824 0.00000";
	}
	else if (value.find("yellowgreen") != string::npos) {
		g4value = "0.60392 0.80392 0.19608";
	}
	else if (value.find("lemonchiffon") != string::npos) {
		g4value = "1.00000 0.98039 0.80392";
	}
	else if (value.find("antiquewhite") != string::npos) {
		g4value = "0.98039 0.92157 0.84314";
	}
	else if (value.find("wheat") != string::npos) {
		g4value = "0.96078 0.87059 0.70196";
	}
	else if (value.find("sienna") != string::npos) {
		g4value = "0.62745 0.32157 0.17647";
	}
	else if (value.find("snow") != string::npos) {
		g4value = "1.00000 0.98039 0.98039";
	}
	else if (value.find("floralwhite") != string::npos) {
		g4value = "1.00000 0.98039 0.94118";
	}
	else if (value.find("lightsalmon") != string::npos) {
		g4value = "1.00000 0.62745 0.47843";
	}
	else if (value.find("orchid") != string::npos) {
		g4value = "0.85490 0.43922 0.83922";
	}
	else if (value.find("plum") != string::npos) {
		g4value = "0.86667 0.62745 0.86667";
	}
	else {
		// Fallback to white for unexpected selections.
		g4value = "1.0 1.0 1.0";
	}

	string command = "/vis/viewer/set/background " + g4value;
	G4UImanager::GetUIpointer()->ApplyCommand(command);
}

void G4DisplayView::changeLightDirection() {
	// Construct the Geant4 command using the current slider values and send it to the UI manager.
	string command = "/vis/viewer/set/lightsThetaPhi " +
		to_string(lightTheta->value()) + " " +
		to_string(lightPhi->value());
	G4UImanager::GetUIpointer()->ApplyCommand(command);
}

void G4DisplayView::setLightDirection(int which) {
	// Dropdown values are interpreted as degrees; command is issued first, then the slider is synced.
	string thetaValue = lthetaDropdown->currentText().toStdString();
	string phiValue   = lphiDropdown->currentText().toStdString();

	string command = "/vis/viewer/set/lightsThetaPhi " + thetaValue + " " + phiValue;
	G4UImanager::GetUIpointer()->ApplyCommand(command);

	int thetaDeg = lthetaDropdown->currentText().toInt();
	int phiDeg   = lphiDropdown->currentText().toInt();

	if (lightTheta && which == 0) lightTheta->setValue(thetaDeg);
	if (lightPhi && which == 1) lightPhi->setValue(phiDeg);
}

void G4DisplayView::slice() {
	G4UImanager* g4uim = G4UImanager::GetUIpointer();
	if (g4uim == nullptr) { return; }

	// Reset existing planes before applying the newly requested slice configuration.
	g4uim->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	// Select how multiple planes combine.
	if (sliceSectn->isChecked()) { g4uim->ApplyCommand("/vis/viewer/set/cutawayMode intersection"); }
	else if (sliceUnion->isChecked()) { g4uim->ApplyCommand("/vis/viewer/set/cutawayMode union"); }

	// Clear again to ensure mode change does not retain previously-defined planes.
	g4uim->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	// For each enabled axis, add a plane at the requested position. Values are interpreted as mm.
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
}

void G4DisplayView::clearSlices() {
	// Clear cutaway planes in the viewer and reset activation UI state.
	G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/clearCutawayPlanes");

	// NOTE: Only X is reset here in the current implementation; Y/Z are left unchanged.
	// This behavior is preserved intentionally (no logic changes).
	sliceXActi->setChecked(false);
}

void G4DisplayView::apply_buttons_set1(int index) {
	G4UImanager* g4uim = G4UImanager::GetUIpointer();
	if (g4uim == nullptr) { return; }

	bool button_state = buttons_set1->lastButtonState();

	if (index == 0) {
		// Hidden edges on/off.
		string command = string("/vis/viewer/set/hiddenEdge") + (button_state ? " 1" : " 0");
		g4uim->ApplyCommand(command);
		g4uim->ApplyCommand("/vis/viewer/flush");
	}
	else if (index == 1) {
		// Anti-aliasing: handled via OpenGL state for viewers where it applies.
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
		// Auxiliary edges typically implies hidden edges; keep UI state coherent.
		string command = string("/vis/viewer/set/auxiliaryEdge") + (button_state ? " 1" : " 0");
		g4uim->ApplyCommand(command);

		command = string("/vis/viewer/set/hiddenEdge") + (button_state ? " 1" : " 0");
		g4uim->ApplyCommand(command);

		if (buttons_set1->buttonStatus(0) != button_state) { buttons_set1->toggleButton(0); }
	}
	else if (index == 3) {
		// Magnetic field line visualization model.
		if (button_state == 0) {
			string command = string("/vis/scene/activateModel Field 0");
			g4uim->ApplyCommand(command);
			g4uim->ApplyCommand("/vis/scene/removeModel Field");
		}
		else {
			string npoints = to_string(field_NPOINTS);
			string command = string("/vis/scene/add/magneticField ") + npoints;
			g4uim->ApplyCommand(command);
		}
	}
	else if (index == 4) {
		// Axes visualization (added when enabled).
		if (button_state == 1) {
			string command = string("/vis/scene/add/axes");
			g4uim->ApplyCommand(command);
		}
	}
	else if (index == 5) {
		// Scale visualization (added when enabled).
		if (button_state == 1) {
			string command = string("/vis/scene/add/scale");
			g4uim->ApplyCommand(command);
		}
	}
}

void G4DisplayView::field_precision_changed() {
	G4UImanager* g4uim = G4UImanager::GetUIpointer();
	if (g4uim == nullptr) { return; }

	// Parse the updated point count from the UI.
	field_NPOINTS = field_npoints->text().toInt();

	// If field lines are currently active, re-issue the field model to apply the new point count.
	if (buttons_set1->buttonStatus(3) == 1) {
		string command = string("vis/scene/activateModel Field 0");
		g4uim->ApplyCommand(command);
		g4uim->ApplyCommand("/vis/scene/removeModel Field");

		string npoints = to_string(field_NPOINTS);
		command        = string("/vis/scene/add/magneticField ") + npoints;
		G4UImanager::GetUIpointer()->ApplyCommand(command);
	}
}
