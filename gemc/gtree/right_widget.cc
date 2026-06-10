// Right-side properties panel construction for GTree.
// Doxygen documentation is authoritative in the header; this file provides a
// short implementation-focused description and inline comments.

#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

#include "gtree.h"

// Build the right-side panel that shows properties and controls for the selected item.
QWidget* GTree::right_widget() {
    // Container is parented to the GTree widget so Qt manages lifetime.
    auto* container = new QWidget(this);
    auto* vlayout = new QVBoxLayout(container);
    vlayout->setContentsMargins(0, 0, 0, 0);
    vlayout->setSpacing(4);

    // Representation buttons live at the very top, always visible.
    std::vector<std::string> bicons;
    bicons.push_back(":/gtree/images/wireframe");
    bicons.push_back(":/gtree/images/surface");
    bicons.push_back(":/gtree/images/cloud");
    bicons.push_back(":/gtree/images/centre");
    styleButtons = new GQTButtonsWidget(96, 96, bicons, false);
    styleButtons->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    vlayout->addWidget(styleButtons);

    // Bottom panel: only visible when a tree item is selected.
    bottomPanel = new QWidget(container);
    auto* blayout = new QVBoxLayout(bottomPanel);
    blayout->setContentsMargins(0, 0, 0, 0);
    blayout->setSpacing(2);

    // Opacity controls: label + slider + numeric value.
    auto* opacityContainer = new QWidget(bottomPanel);
    auto* opacityLayout = new QHBoxLayout(opacityContainer);
    opacityLayout->setContentsMargins(0, 0, 0, 0);

    auto* label = new QLabel(tr("Opacity:"), opacityContainer);
    opacityLabel = new QLabel(tr("1.00"), opacityContainer);

    opacitySlider = new QSlider(Qt::Horizontal, opacityContainer);
    opacitySlider->setRange(0, 100);
    opacitySlider->setValue(100);
    opacitySlider->setSingleStep(5);
    opacitySlider->setPageStep(10);

    opacityLayout->addWidget(label);
    opacityLayout->addWidget(opacitySlider);
    opacityLayout->addWidget(opacityLabel);

    blayout->addWidget(opacityContainer);
    blayout->addSpacing(4);

    // Info labels — all added directly, no scroll bar.
    typeLabel        = new QLabel(bottomPanel);
    daughtersLabel   = new QLabel(bottomPanel);
    nameLabel        = new QLabel(bottomPanel);
    materialLabel    = new QLabel(bottomPanel);
    massLabel        = new QLabel(bottomPanel);
    volumeLabel      = new QLabel(bottomPanel);
    densityLabel     = new QLabel(bottomPanel);
    solidTypeLabel   = new QLabel(bottomPanel);
    positionLabel    = new QLabel(bottomPanel);
    rotationLabel    = new QLabel(bottomPanel);
    motherLabel      = new QLabel(bottomPanel);
    descriptionLabel = new QLabel(bottomPanel);
    descriptionLabel->setWordWrap(true);

    // parametersLabel as a read-only QTextEdit: owns its own vertical scrollbar,
    // expands to show all content, scrolls only when the panel is too short.
    parametersLabel = new QTextEdit(bottomPanel);
    parametersLabel->setReadOnly(true);
    parametersLabel->setFrameShape(QFrame::NoFrame);
    parametersLabel->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    parametersLabel->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    parametersLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    parametersLabel->setStyleSheet(
        "QTextEdit { background-color: rgba(255,255,255,30); border: 1px solid black; border-radius: 4px; padding: 3px; }");

    blayout->addWidget(typeLabel);
    blayout->addWidget(daughtersLabel);
    blayout->addWidget(nameLabel);
    blayout->addWidget(materialLabel);
    blayout->addWidget(massLabel);
    blayout->addWidget(volumeLabel);
    blayout->addWidget(densityLabel);
    blayout->addSpacing(6);
    blayout->addWidget(solidTypeLabel);
    blayout->addWidget(parametersLabel, 1);
    blayout->addWidget(positionLabel);
    blayout->addWidget(rotationLabel);
    blayout->addWidget(motherLabel);
    blayout->addWidget(descriptionLabel);
    blayout->addStretch();

    // Inspect button: opens the selected volume in a dedicated viewer window.
    blayout->addSpacing(10);
    inspectButton = new QPushButton(bottomPanel);
    inspectButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    inspectButton->setStyleSheet(
        "QPushButton { border: 2px solid black; border-radius: 4px; padding: 4px 8px; }"
        "QPushButton:hover { background-color: palette(highlight); color: palette(highlighted-text); }"
    );
    connect(inspectButton, &QPushButton::clicked, this, &GTree::inspectVolume);
    blayout->addWidget(inspectButton);

    // Draw Logical Overlaps button: not yet functional — blocked by Geant4 11.4.1 TOOLSSG bug.
    blayout->addSpacing(4);
    drawOverlapsButton = new QPushButton(bottomPanel);
    drawOverlapsButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    drawOverlapsButton->setStyleSheet(
        "QPushButton { border: 2px solid black; border-radius: 4px; padding: 4px 8px; }"
        "QPushButton:hover { background-color: palette(highlight); color: palette(highlighted-text); }"
    );
    connect(drawOverlapsButton, &QPushButton::clicked, this, &GTree::drawOverlapsWarning);
    blayout->addWidget(drawOverlapsButton);

    // Hidden until a volume or system is selected in the tree.
    bottomPanel->setVisible(false);

    vlayout->addWidget(bottomPanel, 1);

    return container;
}
