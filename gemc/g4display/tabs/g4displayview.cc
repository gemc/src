#include "g4displayview.h"
#include "g4display_options.h"
#include "gutilities.h"

#include <QPainter>

using namespace g4display;

// c++
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;
using namespace gutilities;

// c++ math
#include <cmath>

// geant4
#include "G4UImanager.hh"
#include "G4VisManager.hh"
#include "G4VViewer.hh"

// Implementation note:
// Detailed Doxygen documentation for public behavior and slots is maintained in g4displayview.h (see rule 7).
namespace {
QColor colorFromG4Rgb(const std::string& rgb) {
	std::istringstream stream(rgb);
	double r = 0.05;
	double g = 0.05;
	double b = 0.26;
	stream >> r >> g >> b;
	return QColor::fromRgbF(std::clamp(r, 0.0, 1.0),
	                        std::clamp(g, 0.0, 1.0),
	                        std::clamp(b, 0.0, 1.0));
}

QString g4RgbFromColor(const QColor& color) {
	return QString("%1 %2 %3")
		.arg(color.redF(), 0, 'f', 5)
		.arg(color.greenF(), 0, 'f', 5)
		.arg(color.blueF(), 0, 'f', 5);
}

QIcon colorIcon(const QColor& color, const QColor& border) {
	QPixmap pixmap(18, 18);
	pixmap.fill(Qt::transparent);

	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(color);
	painter.setPen(border);
	painter.drawRoundedRect(pixmap.rect().adjusted(1, 1, -2, -2), 3, 3);
	return QIcon(pixmap);
}

std::string rootExtentForFieldCommand(const std::shared_ptr<GOptions>& gopts) {
	if (gopts == nullptr) { return ""; }

	std::string rootDefinition = gopts->getScalarString("root");
	for (auto& c : rootDefinition) {
		if (c == ',') { c = ' '; }
	}

	const auto tokens = getStringVectorFromString(rootDefinition);
	if (tokens.size() < 5 || tokens[0] != "G4Box") { return ""; }

	const double dx = getG4Number(tokens[1]);
	const double dy = getG4Number(tokens[2]);
	const double dz = getG4Number(tokens[3]);
	if (dx <= 0 || dy <= 0 || dz <= 0) { return ""; }

	std::ostringstream command;
	command << "/vis/set/extentForField "
	        << -dx << " " << dx << " "
	        << -dy << " " << dy << " "
	        << -dz << " " << dz << " mm";
	return command.str();
}

void applyRootExtentForField(const std::shared_ptr<GOptions>& gopts) {
	const auto command = rootExtentForFieldCommand(gopts);
	if (!command.empty()) { G4UImanager::GetUIpointer()->ApplyCommand(command); }
}
}

G4DisplayView::G4DisplayView(const std::shared_ptr<GOptions>& gopts,
                             std::shared_ptr<GLogger>         logger,
                             QWidget*                         parent)
	: QWidget(parent), gopts(gopts), log(logger) {
	log->debug(CONSTRUCTOR, "G4DisplayView");

	// LCD font used to display theta/phi degrees next to sliders.
	QFont flcd;
	flcd.setFamilies({"Helvetica"}); // Qt6: prefer setFamilies over family string ctor
	flcd.setPointSize(24);
	flcd.setBold(true);

	// Initial camera configuration comes from options; values are converted to degrees for UI.
	G4Camera jcamera    = getG4Camera(gopts);
	double   thetaValue = getG4Number(jcamera.theta);
	double   phiValue   = getG4Number(jcamera.phi);
	G4Light jlight       = getG4Light(gopts);
	double  lightThetaValue = getG4Number(jlight.theta);
	double  lightPhiValue   = getG4Number(jlight.phi);
	G4View  g4view = getG4View(gopts);
	backgroundColor = colorFromG4Rgb(g4view.background);
	cloudPoints = g4view.cloudPoints;

	// Toggle buttons for common viewer/scene flags.
	// Button 0 uses an SVG icon (set below); its title is left empty so only the SVG is shown.
	vector<string> toggle_button_titles;
	toggle_button_titles.emplace_back("");
	toggle_button_titles.emplace_back("");
	toggle_button_titles.emplace_back("");
	toggle_button_titles.emplace_back("");

	buttons_set1 = new GQTToggleButtonWidget(120, 120, 0, toggle_button_titles, false, this);
	connect(buttons_set1, &GQTToggleButtonWidget::buttonPressedIndexChanged, this, &G4DisplayView::apply_buttons_set1);

	buttons_set1->setSvgButtonIcon(0, ":/images/hidden_lines.svg",    QSize(120, 120));
	buttons_set1->setSvgButtonIcon(1, ":/images/anti_aliasing.svg",   QSize(120, 120));
	buttons_set1->setSvgButtonIcon(2, ":/images/auxiliary_edges.svg", QSize(120, 120));
	buttons_set1->setSvgButtonIcon(3, ":/images/field_lines.svg",     QSize(120, 120));

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

	thetaLCD = new QLCDNumber(this);
	thetaLCD->setFont(flcd);
	thetaLCD->setMaximumSize(QSize(42, 32));
	thetaLCD->setSegmentStyle(QLCDNumber::Flat);

	thetaDropdown = new QComboBox(this);
	thetaDropdown->addItems(theta_angle_Set);
	thetaDropdown->setMaximumSize(QSize(100, 45));

	auto cameraThetaLayout = new QHBoxLayout;
	cameraThetaLayout->setContentsMargins(0, 0, 0, 0);
	cameraThetaLayout->setSpacing(4);
	cameraThetaLayout->addWidget(cameraThetaLabel);
	cameraThetaLayout->addWidget(cameraTheta);
	cameraThetaLayout->addWidget(thetaLCD);
	cameraThetaLayout->addWidget(thetaDropdown);

	cameraPhi = new QSlider(Qt::Horizontal);
	cameraPhi->setRange(0, 360);
	cameraPhi->setSingleStep(1);
	cameraPhi->setValue(phiValue);
	cameraPhi->setTracking(true); // updates while dragging

	auto cameraPhiLabel = new QLabel(tr("ɸ"));

	phiLCD = new QLCDNumber(this);
	phiLCD->setFont(flcd);
	phiLCD->setMaximumSize(QSize(42, 32));
	phiLCD->setSegmentStyle(QLCDNumber::Flat);

	phiDropdown = new QComboBox(this);
	phiDropdown->addItems(phi_angle_Set);
	phiDropdown->setMaximumSize(QSize(100, 45));

	auto cameraPhiLayout = new QHBoxLayout;
	cameraPhiLayout->setContentsMargins(0, 0, 0, 0);
	cameraPhiLayout->setSpacing(4);
	cameraPhiLayout->addWidget(cameraPhiLabel);
	cameraPhiLayout->addWidget(cameraPhi);
	cameraPhiLayout->addWidget(phiLCD);
	cameraPhiLayout->addWidget(phiDropdown);

	auto* readViewButton = new QPushButton(tr("Read View"), this);
	readViewButton->setToolTip(tr("Sync sliders to current viewer orientation"));
	auto* readViewRow = new QHBoxLayout;
	readViewRow->setContentsMargins(0, 0, 0, 0);
	readViewRow->addStretch(1);
	readViewRow->addWidget(readViewButton);

	QVBoxLayout* cameraDirectionLayout = new QVBoxLayout;
	cameraDirectionLayout->setContentsMargins(6, 6, 6, 6);
	cameraDirectionLayout->setSpacing(2);
	cameraDirectionLayout->addLayout(cameraThetaLayout);
	cameraDirectionLayout->addLayout(cameraPhiLayout);
	cameraDirectionLayout->addLayout(readViewRow);

	QGroupBox* cameraAnglesGroup = new QGroupBox(tr("Camera Direction"));
	cameraAnglesGroup->setLayout(cameraDirectionLayout);

	// Slider -> Geant4 command, and slider -> LCD.
	connect(cameraTheta, &QSlider::valueChanged, this, &G4DisplayView::changeCameraDirection);
	connect(cameraTheta, &QSlider::valueChanged, thetaLCD, qOverload<int>(&QLCDNumber::display));

	// Dropdown -> Geant4 command, and dropdown -> slider sync.
	connect(thetaDropdown, &QComboBox::currentTextChanged,
	        this, [this](const QString&) { setCameraDirection(0); });

	connect(cameraPhi, &QSlider::valueChanged, this, &G4DisplayView::changeCameraDirection);
	connect(cameraPhi, &QSlider::valueChanged, phiLCD, qOverload<int>(&QLCDNumber::display));

	connect(phiDropdown, &QComboBox::currentTextChanged,
	        this, [this](const QString&) { setCameraDirection(1); });

	connect(readViewButton, &QPushButton::clicked, this, &G4DisplayView::readCameraFromViewer);

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
	resolutionAndPerspectiveLayout->setContentsMargins(6, 6, 6, 6);
	resolutionAndPerspectiveLayout->setSpacing(4);
	resolutionAndPerspectiveLayout->addWidget(projLabel);
	resolutionAndPerspectiveLayout->addWidget(perspectiveDropdown);
	resolutionAndPerspectiveLayout->addWidget(sides_per_circlesLabel);
	resolutionAndPerspectiveLayout->addWidget(precisionDropdown);

	QGroupBox* propertyGroup = new QGroupBox(tr("View Properties"));
	propertyGroup->setLayout(resolutionAndPerspectiveLayout);

	QHBoxLayout* cameraAndPerspective = new QHBoxLayout;
	cameraAndPerspective->setContentsMargins(0, 0, 0, 0);
	cameraAndPerspective->setSpacing(6);
	cameraAndPerspective->addWidget(cameraAnglesGroup);
	cameraAndPerspective->addWidget(propertyGroup);

	// -------------------------
	// Light direction controls
	// -------------------------

	lightTheta = new QSlider(Qt::Horizontal);
	lightTheta->setRange(0, 180);
	lightTheta->setSingleStep(1);
	lightTheta->setValue(lightThetaValue);
	lightTheta->setTracking(true);

	auto lightThetaLabel = new QLabel(tr("θ"));

	lthetaLCD = new QLCDNumber(this);
	lthetaLCD->setFont(flcd);
	lthetaLCD->setMaximumSize(QSize(42, 32));
	lthetaLCD->setSegmentStyle(QLCDNumber::Flat);

	lthetaDropdown = new QComboBox(this);
	lthetaDropdown->addItems(theta_angle_Set);
	lthetaDropdown->setMaximumSize(QSize(100, 45));

	auto lightThetaLayout = new QHBoxLayout;
	lightThetaLayout->setContentsMargins(0, 0, 0, 0);
	lightThetaLayout->setSpacing(4);
	lightThetaLayout->addWidget(lightThetaLabel);
	lightThetaLayout->addWidget(lightTheta);
	lightThetaLayout->addWidget(lthetaLCD);
	lightThetaLayout->addWidget(lthetaDropdown);

	lightPhi = new QSlider(Qt::Horizontal);
	lightPhi->setRange(0, 360);
	lightPhi->setSingleStep(1);
	lightPhi->setValue(lightPhiValue);
	lightPhi->setTracking(true);

	auto lightPhiLabel = new QLabel(tr("ɸ"));

	lphiLCD = new QLCDNumber(this);
	lphiLCD->setFont(flcd);
	lphiLCD->setMaximumSize(QSize(42, 32));
	lphiLCD->setSegmentStyle(QLCDNumber::Flat);

	lphiDropdown = new QComboBox(this);
	lphiDropdown->addItems(phi_angle_Set);
	lphiDropdown->setMaximumSize(QSize(100, 45));

	auto lightPhiLayout = new QHBoxLayout;
	lightPhiLayout->setContentsMargins(0, 0, 0, 0);
	lightPhiLayout->setSpacing(4);
	lightPhiLayout->addWidget(lightPhiLabel);
	lightPhiLayout->addWidget(lightPhi);
	lightPhiLayout->addWidget(lphiLCD);
	lightPhiLayout->addWidget(lphiDropdown);

	auto lightDirectionLayout = new QVBoxLayout;
	lightDirectionLayout->setContentsMargins(6, 6, 6, 6);
	lightDirectionLayout->setSpacing(2);
	lightDirectionLayout->addLayout(lightThetaLayout);
	lightDirectionLayout->addLayout(lightPhiLayout);

	QGroupBox* lightAnglesGroup = new QGroupBox(tr("Light Direction"));
	lightAnglesGroup->setLayout(lightDirectionLayout);

	connect(lightTheta, &QSlider::valueChanged, this, &G4DisplayView::changeLightDirection);
	connect(lightTheta, &QSlider::valueChanged, lthetaLCD, qOverload<int>(&QLCDNumber::display));
	connect(lthetaDropdown, &QComboBox::currentTextChanged,
	        this, [this](const QString&) { setLightDirection(0); });

	connect(lightPhi, &QSlider::valueChanged, this, &G4DisplayView::changeLightDirection);
	connect(lightPhi, &QSlider::valueChanged, lphiLCD, qOverload<int>(&QLCDNumber::display));
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
	const auto addBackgroundPreset = [this](const QString& name, const QString& rgb) {
		backgroundColorDropdown->addItem(name, rgb);
	};
	addBackgroundPreset(tr("lightslategray"), "0.46667 0.53333 0.60000");
	addBackgroundPreset(tr("ghostwhite"), "0.97255 0.97255 1.00000");
	addBackgroundPreset(tr("black"), "0.00000 0.00000 0.00000");
	addBackgroundPreset(tr("navy"), "0.00000 0.00000 0.50196");
	addBackgroundPreset(tr("whitesmoke"), "0.96078 0.96078 0.96078");
	addBackgroundPreset(tr("lightskyblue"), "0.52941 0.80784 0.98039");
	addBackgroundPreset(tr("deepskyblue"), "0.00000 0.74902 1.00000");
	addBackgroundPreset(tr("lightsteelblue"), "0.69020 0.76863 0.87059");
	addBackgroundPreset(tr("blueviolet"), "0.54118 0.16863 0.88627");
	addBackgroundPreset(tr("turquoise"), "0.25098 0.87843 0.81569");
	addBackgroundPreset(tr("mediumaquamarine"), "0.40000 0.80392 0.66667");
	addBackgroundPreset(tr("springgreen"), "0.00000 1.00000 0.49804");
	addBackgroundPreset(tr("lawngreen"), "0.48627 0.98824 0.00000");
	addBackgroundPreset(tr("yellowgreen"), "0.60392 0.80392 0.19608");
	addBackgroundPreset(tr("lemonchiffon"), "1.00000 0.98039 0.80392");
	addBackgroundPreset(tr("antiquewhite"), "0.98039 0.92157 0.84314");
	addBackgroundPreset(tr("wheat"), "0.96078 0.87059 0.70196");
	addBackgroundPreset(tr("sienna"), "0.62745 0.32157 0.17647");
	addBackgroundPreset(tr("snow"), "1.00000 0.98039 0.98039");
	addBackgroundPreset(tr("floralwhite"), "1.00000 0.98039 0.94118");
	addBackgroundPreset(tr("lightsalmon"), "1.00000 0.62745 0.47843");
	addBackgroundPreset(tr("orchid"), "0.85490 0.43922 0.83922");
	addBackgroundPreset(tr("plum"), "0.86667 0.62745 0.86667");
	setBackgroundDropdownColor(backgroundColor);

	backgroundColorButton = new QToolButton(this);
	backgroundColorButton->setToolTip(tr("Choose background color"));
	backgroundColorButton->setAutoRaise(true);
	backgroundColorButton->setIconSize(QSize(18, 18));
	setBackgroundButtonColor(backgroundColor);

	auto* cloudPointsLabel = new QLabel(tr("Number of Cloud Points:"));
	cloudPointsSpinBox = new QSpinBox(this);
	cloudPointsSpinBox->setRange(1, 100000000);
	cloudPointsSpinBox->setSingleStep(100);
	cloudPointsSpinBox->setGroupSeparatorShown(true);
	cloudPointsSpinBox->setValue(std::max(1, cloudPoints));

	auto* explodeLabel = new QLabel(tr("Explode Factor:"));
	explodeSlider = new QSlider(Qt::Horizontal);
	explodeSlider->setRange(0, 100);
	explodeSlider->setValue(0);
	explodeSlider->setSingleStep(1);
	explodeSlider->setPageStep(5);
	explodeValueLabel = new QLabel(tr("1.00"));
	explodeIntensityDropdown = new QComboBox;
	explodeIntensityDropdown->addItem(tr("Low"),    QVariant(45.0));
	explodeIntensityDropdown->addItem(tr("Medium"), QVariant(15.0));
	explodeIntensityDropdown->addItem(tr("High"),   QVariant(5.0));
	explodeIntensityDropdown->setCurrentIndex(1);

	connect(cullingDropdown, &QComboBox::currentTextChanged, this, &G4DisplayView::set_culling);
	connect(backgroundColorDropdown, &QComboBox::currentTextChanged, this, &G4DisplayView::set_background);
	connect(backgroundColorButton, &QToolButton::clicked, this, &G4DisplayView::choose_background_color);
	connect(cloudPointsSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &G4DisplayView::set_cloud_points);
	connect(explodeSlider, &QSlider::valueChanged, this, &G4DisplayView::set_explode);
	connect(explodeIntensityDropdown, &QComboBox::currentIndexChanged, this, &G4DisplayView::set_explode);

	QVBoxLayout* sceneLayout = new QVBoxLayout;
	sceneLayout->setContentsMargins(6, 6, 6, 6);
	sceneLayout->setSpacing(4);
	sceneLayout->addWidget(cullingLabel);
	sceneLayout->addWidget(cullingDropdown);
	sceneLayout->addWidget(backgroundColorLabel);
	auto* backgroundLayout = new QHBoxLayout;
	backgroundLayout->setContentsMargins(0, 0, 0, 0);
	backgroundLayout->setSpacing(4);
	backgroundLayout->addWidget(backgroundColorDropdown);
	backgroundLayout->addWidget(backgroundColorButton);
	sceneLayout->addLayout(backgroundLayout);
	sceneLayout->addWidget(cloudPointsLabel);
	sceneLayout->addWidget(cloudPointsSpinBox);
	sceneLayout->addWidget(explodeLabel);
	auto* explodeRow = new QHBoxLayout;
	explodeRow->setContentsMargins(0, 0, 0, 0);
	explodeRow->setSpacing(4);
	explodeRow->addWidget(explodeSlider);
	explodeRow->addWidget(explodeValueLabel);
	explodeRow->addWidget(explodeIntensityDropdown);
	sceneLayout->addLayout(explodeRow);

	QGroupBox* spropertyGroup = new QGroupBox(tr("Scene Properties"));
	spropertyGroup->setLayout(sceneLayout);

	QHBoxLayout* lightAndProperties = new QHBoxLayout;
	lightAndProperties->setContentsMargins(0, 0, 0, 0);
	lightAndProperties->setSpacing(6);
	lightAndProperties->addWidget(lightAnglesGroup);
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
	sliceXLayout->setContentsMargins(0, 0, 0, 0);
	sliceXLayout->setSpacing(4);
	sliceXLayout->addWidget(new QLabel(tr("X: ")));
	sliceXLayout->addWidget(sliceXEdit);
	sliceXLayout->addSpacing(16);
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
	sliceYLayout->setContentsMargins(0, 0, 0, 0);
	sliceYLayout->setSpacing(4);
	sliceYLayout->addWidget(new QLabel(tr("Y: ")));
	sliceYLayout->addWidget(sliceYEdit);
	sliceYLayout->addSpacing(16);
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
	sliceZLayout->setContentsMargins(0, 0, 0, 0);
	sliceZLayout->setSpacing(4);
	sliceZLayout->addWidget(new QLabel(tr("Z: ")));
	sliceZLayout->addWidget(sliceZEdit);
	sliceZLayout->addSpacing(16);
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
	sliceChoiceLayout->setContentsMargins(6, 4, 6, 4);
	sliceChoiceLayout->setSpacing(8);
	sliceChoiceLayout->addWidget(sliceSectn);
	sliceChoiceLayout->addWidget(sliceUnion);
	sliceChoiceBox->setLayout(sliceChoiceLayout);

	// Slices layout.
	auto sliceLayout = new QVBoxLayout;
	sliceLayout->setContentsMargins(0, 0, 0, 0);
	sliceLayout->setSpacing(3);
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
	font.setPointSize(18);
	field_npoints->setFont(font);

	connect(field_npoints, &QLineEdit::returnPressed, this, &G4DisplayView::field_precision_changed);

	// Buttons + field point count UI combined.
	auto fieldPointsHBox = new QHBoxLayout;
	fieldPointsHBox->setContentsMargins(6, 4, 6, 4);
	fieldPointsHBox->setSpacing(4);
	fieldPointsHBox->addWidget(field_npoints);
	fieldPrecisionBox->setLayout(fieldPointsHBox);

	auto buttons_field_HBox = new QHBoxLayout;
	buttons_field_HBox->setContentsMargins(0, 0, 0, 0);
	buttons_field_HBox->setSpacing(6);
	buttons_field_HBox->addWidget(buttons_set1);
	buttons_field_HBox->addWidget(fieldPrecisionBox);
	fieldPrecisionBox->setMaximumHeight(3 * buttons_set1->height());
	fieldPrecisionBox->setMaximumWidth(140);

	// -------------------------
	// Assemble final tab layout
	// -------------------------

	auto mainLayout = new QVBoxLayout;
	mainLayout->setContentsMargins(6, 6, 6, 6);
	mainLayout->setSpacing(6);
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

void G4DisplayView::syncViewToOptions() {
	if (!gopts) return;
	const auto g4view = getG4View(gopts);
	const QString s = QString("{driver: \"%1\", dimension: \"%2\", position: \"%3\", "
	                          "segsPerCircle: %4, background: \"%5\", cloudPoints: %6}")
	    .arg(QString::fromStdString(g4view.driver))
	    .arg(QString::fromStdString(g4view.dimension))
	    .arg(QString::fromStdString(g4view.position))
	    .arg(g4view.segsPerCircle)
	    .arg(g4RgbFromColor(backgroundColor))
	    .arg(cloudPoints);
	gopts->setOptionValueFromString("g4view", s.toStdString());
}

void G4DisplayView::setBackgroundButtonColor(const QColor& color) {
	if (!backgroundColorButton) { return; }

	backgroundColorButton->setIcon(colorIcon(color, palette().color(QPalette::Mid)));
}

void G4DisplayView::setBackgroundDropdownColor(const QColor& color) {
	if (!backgroundColorDropdown) { return; }

	QSignalBlocker blocker(backgroundColorDropdown);
	const QString wanted = g4RgbFromColor(color);
	for (int i = 0; i < backgroundColorDropdown->count(); ++i) {
		const QColor candidate = colorFromG4Rgb(backgroundColorDropdown->itemData(i).toString().toStdString());
		if (qAbs(candidate.redF() - color.redF()) < 0.0001 &&
		    qAbs(candidate.greenF() - color.greenF()) < 0.0001 &&
		    qAbs(candidate.blueF() - color.blueF()) < 0.0001) {
			backgroundColorDropdown->setCurrentIndex(i);
			return;
		}
	}

	backgroundColorDropdown->addItem(tr("custom"), wanted);
	backgroundColorDropdown->setCurrentIndex(backgroundColorDropdown->count() - 1);
}

void G4DisplayView::set_background() {
	const QString g4value = backgroundColorDropdown->currentData().toString();
	if (g4value.isEmpty()) { return; }

	backgroundColor = colorFromG4Rgb(g4value.toStdString());
	setBackgroundButtonColor(backgroundColor);

	string command = "/vis/viewer/set/background " + g4value.toStdString();
	G4UImanager::GetUIpointer()->ApplyCommand(command);
	syncViewToOptions();
}

void G4DisplayView::choose_background_color() {
	const QColor selected = QColorDialog::getColor(backgroundColor, this, tr("Select background color"));
	if (!selected.isValid()) { return; }

	backgroundColor = selected;
	setBackgroundButtonColor(backgroundColor);
	setBackgroundDropdownColor(backgroundColor);

	const QString g4value = g4RgbFromColor(backgroundColor);
	const string command = "/vis/viewer/set/background " + g4value.toStdString();
	G4UImanager::GetUIpointer()->ApplyCommand(command);
	syncViewToOptions();
}

void G4DisplayView::set_cloud_points() {
	if (!cloudPointsSpinBox) { return; }

	cloudPoints = cloudPointsSpinBox->value();
	const string command = "/vis/viewer/set/numberOfCloudPoints " + to_string(cloudPoints);
	G4UImanager::GetUIpointer()->ApplyCommand(command);
	syncViewToOptions();
}

void G4DisplayView::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);
	set_background();
	set_cloud_points();
	readCameraFromViewer();

	// Light direction: use g4light values if specified, otherwise follow camera.
	const auto   g4light       = getG4Light(gopts);
	const double toDegrees     = 180.0 / M_PI;
	const double lightThetaDeg = getG4Number(g4light.theta) * toDegrees;
	const double lightPhiDeg   = getG4Number(g4light.phi)   * toDegrees;

	const int lt = (lightThetaDeg == 0.0 && lightPhiDeg == 0.0)
	               ? cameraTheta->value()
	               : static_cast<int>(std::round(lightThetaDeg));
	const int lp = (lightThetaDeg == 0.0 && lightPhiDeg == 0.0)
	               ? cameraPhi->value()
	               : static_cast<int>(std::round(lightPhiDeg));

	QSignalBlocker blt(lightTheta);
	QSignalBlocker blp(lightPhi);
	lightTheta->setValue(lt);
	lightPhi->setValue(lp);
	lthetaLCD->display(lt);
	lphiLCD->display(lp);
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
			applyRootExtentForField(gopts);
			string command = string("/vis/scene/add/magneticField ") + npoints;
			g4uim->ApplyCommand(command);
		}
	}
}

void G4DisplayView::set_explode() {
	if (!explodeSlider) return;

	const double divisor = (explodeIntensityDropdown && explodeIntensityDropdown->currentData().toDouble() > 0)
	    ? explodeIntensityDropdown->currentData().toDouble()
	    : 15.0;
	const int    boom = explodeSlider->value();
	const double xf   = 1.0 + static_cast<double>(boom) / divisor;

	if (explodeValueLabel)
		explodeValueLabel->setText(QString::number(xf, 'f', 2));

	G4UImanager* g4uim = G4UImanager::GetUIpointer();
	if (!g4uim) return;
	g4uim->ApplyCommand("/vis/viewer/set/explodeFactor " + QString::number(xf, 'f', 2).toStdString());
	g4uim->ApplyCommand("/vis/viewer/flush");
}

void G4DisplayView::readCameraFromViewer() {
	auto* vm = G4VisManager::GetInstance();
	if (!vm) return;
	const auto* viewer = vm->GetCurrentViewer();
	if (!viewer) return;

	const G4Vector3D& vp = viewer->GetViewParameters().GetViewpointDirection();

	const double cosTheta = std::clamp(vp.z(), -1.0, 1.0);
	const double thetaDeg = std::acos(cosTheta) * 180.0 / M_PI;
	double phiDeg = std::atan2(vp.y(), vp.x()) * 180.0 / M_PI;
	if (phiDeg < 0.0) phiDeg += 360.0;

	const int thetaInt = static_cast<int>(std::round(thetaDeg));
	const int phiInt   = static_cast<int>(std::round(phiDeg));

	QSignalBlocker bt(cameraTheta);
	QSignalBlocker bp(cameraPhi);
	cameraTheta->setValue(thetaInt);
	cameraPhi->setValue(phiInt);
	thetaLCD->display(thetaInt);
	phiLCD->display(phiInt);
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
		applyRootExtentForField(gopts);
		command        = string("/vis/scene/add/magneticField ") + npoints;
		G4UImanager::GetUIpointer()->ApplyCommand(command);
	}
}
