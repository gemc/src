#include <QLabel>
#include <QVBoxLayout>

#include "gtree.h"

QWidget* GTree::right_widget() {
    auto* container = new QWidget(this);
    auto* vlayout = new QVBoxLayout(container);

    // Top widget: always present
    auto* topLabel = new QLabel(tr("Properties"), container);
    QFont f = topLabel->font();
    f.setBold(true);
    topLabel->setFont(f);
    vlayout->addWidget(topLabel);

    // Bottom widget: only visible when a tree item is pressed
    bottomPanel = new QWidget(container);
    auto* blayout = new QVBoxLayout(bottomPanel);

    std::vector<std::string> bicons;
    bicons.push_back(":/gtree/images/wireframe");
    bicons.push_back(":/gtree/images/surface");
    bicons.push_back(":/gtree/images/cloud");
    styleButtons = new GQTButtonsWidget(96, 96, bicons, false);
    blayout->addWidget(styleButtons, 1);

    auto* opacityContainer = new QWidget(bottomPanel);
    auto* opacityLayout = new QHBoxLayout(opacityContainer);
    opacityLayout->setContentsMargins(0, 0, 0, 0);

    auto* label = new QLabel(tr("Opacity:"), opacityContainer);
    opacityLabel = new QLabel(tr("1.00"), opacityContainer); // default text

    opacitySlider = new QSlider(Qt::Horizontal, opacityContainer);
    opacitySlider->setRange(0, 100); // 0 → 0.0, 100 → 1.0
    opacitySlider->setValue(100); // default fully opaque
    opacitySlider->setSingleStep(5);
    opacitySlider->setPageStep(10);

    opacityLayout->addWidget(label);
    opacityLayout->addWidget(opacitySlider);
    opacityLayout->addWidget(opacityLabel);


    // Info labels
    typeLabel = new QLabel(bottomPanel);
    daughtersLabel = new QLabel(bottomPanel);
    nameLabel = new QLabel(bottomPanel);
    materialLabel = new QLabel(bottomPanel);
    massLabel = new QLabel(bottomPanel);
    volumeLabel = new QLabel(bottomPanel);
    densityLabel = new QLabel(bottomPanel);

    blayout->addWidget(opacityContainer);
    blayout->addSpacing(10);
    blayout->addWidget(typeLabel);
    blayout->addWidget(daughtersLabel);
    blayout->addWidget(nameLabel);
    blayout->addWidget(materialLabel);
    blayout->addWidget(massLabel);
    blayout->addWidget(volumeLabel);
    blayout->addWidget(densityLabel);
    blayout->addStretch();

    bottomPanel->setVisible(false); // hidden until a tree item is clicked

    vlayout->addWidget(bottomPanel);
    vlayout->addStretch();

    return container;
}
