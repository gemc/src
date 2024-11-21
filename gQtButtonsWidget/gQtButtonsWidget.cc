// gQtButtonsWidget 
#include "gQtButtonsWidget.h"

using namespace std;

// ButtonInfo constructor
ButtonInfo::ButtonInfo(string icon) : buttonName(icon) {
    thisButton = new QListWidgetItem();

    // default state is normal
    thisButton->setIcon(iconForState(1));

    // a second click on the same button will unselect the button
    // thisButton->setCheckState(Qt::Unchecked);

    thisButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

}

// returns icon depending on state
QIcon ButtonInfo::iconForState(int state) {
    string bname = buttonName + "_" + to_string(state) + ".png";

    QFileInfo checkFile(QString(bname.c_str()));

    // check if file exists and if yes: Is it really a file and no directory?
    if (checkFile.exists() && checkFile.isFile()) {
        return QIcon(QString(bname.c_str()));
    }

    return QIcon();
}


// GQTButtonsWidget constructor
GQTButtonsWidget::GQTButtonsWidget(double h, double v, vector <string> bicons, bool is_vertical, QWidget *parent) :
        QWidget(parent) {

    static int distanceToMargin = 12;

    buttons.clear();
    for (auto &b: bicons) {
        buttons.push_back(new ButtonInfo(b));
    }

    buttonsWidget = new QListWidget;
    buttonsWidget->setViewMode(QListView::IconMode);
    buttonsWidget->setIconSize(QSize(h, v));
    //buttonsWidget->setMovement(QListView::Static);

    // transparent background
    buttonsWidget->setStyleSheet("background-color: rgba(0,0,0,0);");

    for (auto &b: buttons) {
        buttonsWidget->addItem(b->thisButton);
    }

    connect(buttonsWidget, SIGNAL(itemPressed(QListWidgetItem * )), this, SLOT(buttonWasPressed(QListWidgetItem * )));

    QBoxLayout *layout = nullptr;
    if (is_vertical) {
        layout = new QVBoxLayout(this);
    } else {
        layout = new QHBoxLayout(this);
    }

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(buttonsWidget);
    setLayout(layout);

    // icon container sizes
    // depends on the OS
    // on linux platformName returns xcb
    // see also QGuiApplication Class, property platformName
    //	if( QGuiApplication::platformName().toStdString() == "cocoa")
    //	buttonsWidget->setFixedSize(76, 600);
    //	else
    //	buttonsWidget->setFixedSize(74, 620);

    // remove scrollbars and fit content to buttons
    buttonsWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    buttonsWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    double hsize = (h + distanceToMargin) * (buttons.size());
    double vsize = v + distanceToMargin;

    if (is_vertical) {
        hsize = h + distanceToMargin;
        vsize = (v + distanceToMargin) * (buttons.size());
    }

    buttonsWidget->setFixedSize(hsize, vsize);
}


void GQTButtonsWidget::buttonWasPressed(QListWidgetItem *item) {
    for (int i = 0; i < buttonsWidget->count(); i++) { buttonsWidget->item(i)->setIcon(buttons[i]->iconForState(1)); }

    // starts at 0
    int index = buttonsWidget->currentRow();

    item->setIcon(buttons[index]->iconForState(2));
}

GQTToggleButtonWidget::GQTToggleButtonWidget(int buttonWidth, int buttonHeight, int borderRadius,
                                             std::vector <std::string> titles, bool vertical, QWidget *parent) :
        QWidget(parent), buttonPressedIndex(-1) {

    // Create a vertical layout
    QBoxLayout *layout = nullptr;
    if (vertical) {
        layout = new QVBoxLayout(this);
    } else {
        layout = new QHBoxLayout(this);
    }

    // Strings for the toggle buttons
    QStringList buttonStrings;
    for (const std::string &title: titles) {
        buttonStrings.append(QString::fromStdString(title));
    }

    // Create buttons and add them to the layout
    for (int i = 0; i < buttonStrings.size(); ++i) {

        QPushButton *button = new QPushButton(buttonStrings[i], this);
        button->setCheckable(true); // Make the button toggleable
        button->setFixedSize(buttonWidth, buttonHeight); // Set button size

//        QString imagePath = "g4display/images/hidden_lines.png";
//        QPixmap pixmap(imagePath);
//        button->setIcon(QIcon(pixmap));
//        button->setIconSize(button->size());


        // Apply stylesheet for round shape and dynamic color
        button->setStyleSheet(QString(
                "QPushButton {"
                "    border-radius: %1px;" // Use the borderRadius parameter
                "    border: 2px solid black;"
                "    background-color: rgba(255, 0, 0, 150);"
                "    font-weight: bold;"
                "}"
                "QPushButton:checked {"
                "    background-color: rgba(0, 255, 0, 150);"

                "}"
        ).arg(borderRadius)); // Use the borderRadius parameter);
        layout->addWidget(button);

        buttons.push_back(button); // Store button for later access

        //  map the button's clicked or toggled signal to its index
        signalMapper->setMapping(button, i);

        // Connect signal mapper to handle button index
        connect(button, &QPushButton::clicked, signalMapper, QOverload<>::of(&QSignalMapper::map));
    }

    // Connect the signal mapper to the slot that updates the index
    connect(signalMapper, &QSignalMapper::mappedInt, this, &GQTToggleButtonWidget::setButtonPressed);


    // Set the layout for this widget
    setLayout(layout);
}
