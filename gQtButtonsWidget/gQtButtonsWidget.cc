// gQtButtonsWidget 
#include "gQtButtonsWidget.h"

using namespace std;

// ButtonInfo constructor
ButtonInfo::ButtonInfo(string icon) : buttonName(icon) {
    thisButton = new QListWidgetItem();

    // default state is normal
    thisButton->setIcon(iconForState(1));

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
GQTButtonsWidget::GQTButtonsWidget(double h, double v, vector <string> bicons, bool vertical, QWidget *parent) :
        QWidget(parent),
        is_vertical(vertical) {

    static int distanceToMargin = 12;

    buttons.clear();
    for (auto &b: bicons) {
        buttons.push_back(new ButtonInfo(b));
    }

    buttonsWidget = new QListWidget;
    buttonsWidget->setViewMode(QListView::IconMode);
    buttonsWidget->setIconSize(QSize(h, v));
    buttonsWidget->setMovement(QListView::Static);
    //buttonsWidget->setMouseTracking(1);

    for (auto &b: buttons) {
        buttonsWidget->addItem(b->thisButton);
    }

    // maybe call from mother
    // buttonsWidget->setCurrentRow(1);
    connect(buttonsWidget, SIGNAL(itemPressed(QListWidgetItem * )), this, SLOT(buttonWasPressed(QListWidgetItem * )));


    // apparently there's no difference between QHBoxLayout and QVBoxLayout?

//	QVBoxLayout *layout = new QVBoxLayout;
//	layout->setContentsMargins(0, 0, 0, 0);
//	layout->addWidget(buttonsWidget);
//	setLayout(layout);

    QHBoxLayout *layout = new QHBoxLayout;
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
