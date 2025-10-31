#include "g4displayview.h"
#include "g4display_options.h"
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
G4DisplayView::G4DisplayView(const std::shared_ptr<GOptions>& gopts, std::shared_ptr<GLogger> logger,
                             QWidget* parent) : QWidget(parent), log(logger) {
    log->debug(CONSTRUCTOR, "G4DisplayView");

    // LCD font
    QFont flcd;
    flcd.setFamilies({"Helvetica"}); // Qt6: prefer setFamilies over family string ctor
    flcd.setPointSize(32);
    flcd.setBold(true);

    G4Camera jcamera = getG4Camera(gopts);

    double thetaValue = getG4Number(jcamera.theta);
    double phiValue = getG4Number(jcamera.phi);

    vector<string> toggle_button_titles;
    toggle_button_titles.emplace_back("Hidden\nLines");
    toggle_button_titles.emplace_back("Anti\nAliasing");
    toggle_button_titles.emplace_back("Auxiliary\nEdges");
    toggle_button_titles.emplace_back("Field\nLines");
    toggle_button_titles.emplace_back("Axes");
    toggle_button_titles.emplace_back("Scale");


    buttons_set1 = new GQTToggleButtonWidget(80, 80, 20, toggle_button_titles, false, this);
    connect(buttons_set1, &GQTToggleButtonWidget::buttonPressedIndexChanged, this, &G4DisplayView::apply_buttons_set1);


    QStringList theta_angle_Set;
    for (int t = 0; t <= 180; t += 30) { theta_angle_Set << QString::number(t); }
    QStringList phi_angle_Set;
    for (int t = 0; t <= 360; t += 30) { phi_angle_Set << QString::number(t); }

    // Camera sliders
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

    connect(cameraTheta, &QSlider::valueChanged, this, &G4DisplayView::changeCameraDirection);
    connect(cameraTheta, &QSlider::valueChanged, theta_LCD, qOverload<int>(&QLCDNumber::display));
    connect(thetaDropdown, &QComboBox::currentTextChanged,
            this, [this](const QString&) {
                setCameraDirection(0); // 0 = theta changed
            });

    connect(cameraPhi, &QSlider::valueChanged, this, &G4DisplayView::changeCameraDirection);
    connect(cameraPhi, &QSlider::valueChanged, phi_LCD, qOverload<int>(&QLCDNumber::display));
    connect(phiDropdown, &QComboBox::currentTextChanged,
            this, [this](const QString&) {
                setCameraDirection(1); // 1 = phi changed
            });


    // projection: Orthogonal / Perspective
    QLabel* projLabel = new QLabel(tr("Projection:"));
    perspectiveDropdown = new QComboBox;
    perspectiveDropdown->addItem(tr("Orthogonal"));
    perspectiveDropdown->addItem(tr("Perspective 30"));
    perspectiveDropdown->addItem(tr("Perspective 45"));
    perspectiveDropdown->addItem(tr("Perspective 60"));

    QLabel* sides_per_circlesLabel = new QLabel(tr("Sides per circle:"));
    precisionDropdown = new QComboBox;
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


    // Light Direction
    lightTheta = new QSlider(Qt::Horizontal);
    lightTheta->setRange(0, 180);
    lightTheta->setSingleStep(1);
    lightTheta->setValue(thetaValue);
    lightTheta->setTracking(true); // updates while dragging
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
    lightPhi->setTracking(true); // updates while dragging
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
            this, [this](const QString&) {
                setLightDirection(0); // 0 = theta changed
            });

    connect(lightPhi, &QSlider::valueChanged, this, &G4DisplayView::changeLightDirection);
    connect(lightPhi, &QSlider::valueChanged, lphi_LCD, qOverload<int>(&QLCDNumber::display));
    connect(lphiDropdown, &QComboBox::currentTextChanged,
            this, [this](const QString&) {
                setLightDirection(1); // 0 = phi changed
            });


    // projection: Orthogonal / Perspective
    QLabel* cullingLabel = new QLabel(tr("Culling:"));
    cullingDropdown = new QComboBox;
    cullingDropdown->addItem(tr("Reset"));
    cullingDropdown->addItem(tr("Covered Daughters"));
    cullingDropdown->addItem(tr("Density: 1 mg/cm3"));
    cullingDropdown->addItem(tr("Density: 10 mg/cm3"));
    cullingDropdown->addItem(tr("Density: 100 mg/cm3"));
    cullingDropdown->addItem(tr("Density: 1 g/cm3"));
    cullingDropdown->addItem(tr("Density: 10 g/cm3"));


    QLabel* backgroundColorLabel = new QLabel(tr("Background Color:"));
    backgroundColorDropdown = new QComboBox;
    backgroundColorDropdown->addItem(tr("ghostwhite"));
    backgroundColorDropdown->addItem(tr("black"));
    backgroundColorDropdown->addItem(tr("navy"));
    backgroundColorDropdown->addItem(tr("lightslategray"));
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


#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)          // Qt ≤6.6
    connect(sliceXActi, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
    connect(sliceYActi, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
    connect(sliceZActi, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
    connect(sliceXInve, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
    connect(sliceYInve, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
    connect(sliceZInve, &QCheckBox::stateChanged, this, &G4DisplayView::slice);
#else                                               // Qt≥6.7
    connect(sliceXActi, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
    connect(sliceYActi, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
    connect(sliceZActi, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
    connect(sliceXInve, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
    connect(sliceYInve, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
    connect(sliceZInve, &QCheckBox::checkStateChanged, this, &G4DisplayView::slice);
#endif


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
    mainLayout->addLayout(cameraAndPerspective);
    mainLayout->addLayout(lightAndProperties);
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
    // Construct the command using the current slider values  and send to the Geant4 UImanager.
    string command = "/vis/viewer/set/viewpointThetaPhi " +
        to_string(cameraTheta->value()) + " " +
        to_string(cameraPhi->value());
    G4UImanager::GetUIpointer()->ApplyCommand(command);
}

void G4DisplayView::setCameraDirection(int which) {
    string thetaValue = thetaDropdown->currentText().toStdString();
    string phiValue = phiDropdown->currentText().toStdString();

    // Construct the command using the current slider values and send to the Geant4 UImanager.
    string command = "/vis/viewer/set/viewpointThetaPhi " + thetaValue + " " + phiValue;
    G4UImanager::GetUIpointer()->ApplyCommand(command);

    int thetaDeg = thetaDropdown->currentText().toInt();
    int phiDeg = phiDropdown->currentText().toInt();

    if (cameraTheta && which == 0)
        cameraTheta->setValue(thetaDeg);
    if (cameraPhi && which == 1)
        cameraPhi->setValue(phiDeg);
}

void G4DisplayView::set_projection() {
    string value = perspectiveDropdown->currentText().toStdString();

    string g4perspective = "o";
    string g4perpvalue = "0";
    if (value.find("Perspective") != string::npos) {
        g4perspective = "p";
        // g4perpvalue is the second item of value separate by space
        g4perpvalue = value.substr(value.find(" ") + 1);
    }


    // Construct the command using the current dropdown values and send to the Geant4 UImanager.
    string command = "/vis/viewer/set/projection " + g4perspective + " " + g4perpvalue;
    G4UImanager::GetUIpointer()->ApplyCommand(command);
}

void G4DisplayView::set_precision() {
    string value = precisionDropdown->currentText().toStdString();

    // Construct the command using the current dropdown values and send to the Geant4 UImanager.
    string command = "/vis/viewer/set/lineSegmentsPerCircle " + value;
    G4UImanager::GetUIpointer()->ApplyCommand(command);
    G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/flush");
}

void G4DisplayView::set_culling() {
    string value = cullingDropdown->currentText().toStdString();
    int index = cullingDropdown->currentIndex() - 2;

    if (value.find("Reset") != string::npos) {
        G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/culling global true");
        G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/culling density false");
    }
    else if (value.find("Daughters") != string::npos) {
        G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/culling coveredDaughters true");
        G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/set/culling density false");
    }
    else {
        vector<double> density = {0.001, 0.01, 0.1, 1.0, 10.0, 100.0, 1000.0};
        string command = "/vis/viewer/set/culling density true " + to_string(density[index]);
        G4UImanager::GetUIpointer()->ApplyCommand(command);
        G4UImanager::GetUIpointer()->ApplyCommand("/vis/viewer/flush");
        log->info(0, command);
    }
}

void G4DisplayView::set_background() {
    string value = backgroundColorDropdown->currentText().toStdString();

    string g4value = "0 0 0";
    if (value.find("black") != string::npos) {
        // CSS “black” = #000000
        g4value = "0.0 0.0 0.0";
    }
    else if (value.find("navy") != string::npos) {
        // CSS “navy” = #000080 → normalized (0,0,128/255 ≈0.50196)
        g4value = "0.0 0.0 0.50196";
    }
    else if (value.find("lightslategray") != string::npos) {
        // #778899 → (119/255,136/255,153/255) ≈ (0.46667,0.53333,0.6)
        g4value = "0.46667 0.53333 0.60000";
    }
    else if (value.find("whitesmoke") != string::npos) {
        // #F5F5F5 → (~0.96078,0.96078,0.96078)
        g4value = "0.96078 0.96078 0.96078";
    }
    else if (value.find("ghostwhite") != string::npos) {
        // #F8F8FF → (0.97255,0.97255,1.0)
        g4value = "0.97255 0.97255 1.00000";
    }
    else if (value.find("lightskyblue") != string::npos) {
        // #87CEFA → (135/255,206/255,250/255) ≈ (0.52941,0.80784,0.98039)
        g4value = "0.52941 0.80784 0.98039";
    }
    else if (value.find("deepskyblue") != string::npos) {
        // #00BFFF → (0,191/255,255/255) ≈ (0.0,0.74902,1.0)
        g4value = "0.00000 0.74902 1.00000";
    }
    else if (value.find("lightsteelblue") != string::npos) {
        // #B0C4DE → (176/255,196/255,222/255) ≈ (0.69020,0.76863,0.87059)
        g4value = "0.69020 0.76863 0.87059";
    }
    else if (value.find("blueviolet") != string::npos) {
        // #8A2BE2 → (138/255,43/255,226/255) ≈ (0.54118,0.16863,0.88627)
        g4value = "0.54118 0.16863 0.88627";
    }
    else if (value.find("turquoise") != string::npos) {
        // #40E0D0 → (64/255,224/255,208/255) ≈ (0.25098,0.87843,0.81569)
        g4value = "0.25098 0.87843 0.81569";
    }
    else if (value.find("mediumaquamarine") != string::npos) {
        // #66CDAA → (102/255,205/255,170/255) ≈ (0.40000,0.80392,0.66667)
        g4value = "0.40000 0.80392 0.66667";
    }
    else if (value.find("springgreen") != string::npos) {
        // #00FF7F → (0,255/255,127/255) ≈ (0.0,1.0,0.49804)
        g4value = "0.00000 1.00000 0.49804";
    }
    else if (value.find("lawngreen") != string::npos) {
        // #7CFC00 → (124/255,252/255,0) ≈ (0.48627,0.98824,0.0)
        g4value = "0.48627 0.98824 0.00000";
    }
    else if (value.find("yellowgreen") != string::npos) {
        // #9ACD32 → (154/255,205/255,50/255) ≈ (0.60392,0.80392,0.19608)
        g4value = "0.60392 0.80392 0.19608";
    }
    else if (value.find("lemonchiffon") != string::npos) {
        // #FFFACD → (255/255,250/255,205/255) ≈ (1.0,0.98039,0.80392)
        g4value = "1.00000 0.98039 0.80392";
    }
    else if (value.find("antiquewhite") != string::npos) {
        // #FAEBD7 → (250/255,235/255,215/255) ≈ (0.98039,0.92157,0.84314)
        g4value = "0.98039 0.92157 0.84314";
    }
    else if (value.find("wheat") != string::npos) {
        // #F5DEB3 → (245/255,222/255,179/255) ≈ (0.96078,0.87059,0.70196)
        g4value = "0.96078 0.87059 0.70196";
    }
    else if (value.find("sienna") != string::npos) {
        // #A0522D → (160/255,82/255,45/255) ≈ (0.62745,0.32157,0.17647)
        g4value = "0.62745 0.32157 0.17647";
    }
    else if (value.find("snow") != string::npos) {
        // #FFFAFA → (255/255,250/255,250/255) ≈ (1.0,0.98039,0.98039)
        g4value = "1.00000 0.98039 0.98039";
    }
    else if (value.find("floralwhite") != string::npos) {
        // #FFFAF0 → (255/255,250/255,240/255) ≈ (1.0,0.98039,0.94118)
        g4value = "1.00000 0.98039 0.94118";
    }
    else if (value.find("lightsalmon") != string::npos) {
        // #FFA07A → (255/255,160/255,122/255) ≈ (1.0,0.62745,0.47843)
        g4value = "1.00000 0.62745 0.47843";
    }
    else if (value.find("orchid") != string::npos) {
        // #DA70D6 → (218/255,112/255,214/255) ≈ (0.85490,0.43922,0.83922)
        g4value = "0.85490 0.43922 0.83922";
    }
    else if (value.find("plum") != string::npos) {
        // #DDA0DD → (221/255,160/255,221/255) ≈ (0.86667,0.62745,0.86667)
        g4value = "0.86667 0.62745 0.86667";
    }
    else {
        // fallback: white
        g4value = "1.0 1.0 1.0";
    }

    string command = "/vis/viewer/set/background " + g4value;
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


void G4DisplayView::setLightDirection(int which) {
    string thetaValue = lthetaDropdown->currentText().toStdString();
    string phiValue = lphiDropdown->currentText().toStdString();

    string command = "/vis/viewer/set/lightsThetaPhi " + thetaValue + " " + phiValue;

    G4UImanager::GetUIpointer()->ApplyCommand(command);

    int thetaDeg = lthetaDropdown->currentText().toInt();
    int phiDeg = lphiDropdown->currentText().toInt();

    if (lightTheta && which == 0)
        lightTheta->setValue(thetaDeg);
    if (lightPhi && which == 1)
        lightPhi->setValue(phiDeg);
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
        if (button_state == 1) {
            string command = string("/vis/scene/add/axes");
            g4uim->ApplyCommand(command);
        }
    }
    else if (index == 5) {
        if (button_state == 1) {
            string command = string("/vis/scene/add/scale");
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
