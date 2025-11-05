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

    // --- Placeholders for your custom widgets ---
    // Replace QWidget with GQTButtonsWidget / GQTToggleButtonWidget once you have them.
    QWidget* buttonsWidget = new QWidget(bottomPanel);
    buttonsWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonsWidget->setMinimumHeight(40); // placeholder
    blayout->addWidget(buttonsWidget);


    std::vector<std::string> bicons;
    bicons.push_back(":/gtree/images/wireframe");
    bicons.push_back(":/gtree/images/surface");
    bicons.push_back(":/gtree/images/cloud");
    styleButtons = new GQTButtonsWidget(96, 96, bicons, false, bottomPanel);
    blayout->addWidget(styleButtons, 1);

    // Info labels
    typeLabel = new QLabel(bottomPanel);
    daughtersLabel = new QLabel(bottomPanel);
    nameLabel = new QLabel(bottomPanel);
    materialLabel = new QLabel(bottomPanel);
    massLabel = new QLabel(bottomPanel);
    volumeLabel = new QLabel(bottomPanel);
    densityLabel = new QLabel(bottomPanel);

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
