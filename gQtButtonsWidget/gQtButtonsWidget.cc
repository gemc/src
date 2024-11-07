// gQtButtonsWidget 
#include "gQtButtonsWidget.h"

using namespace std;

// ButtonInfo constructor
ButtonInfo::ButtonInfo(string icon)  : buttonName(icon)
{
	thisButton = new QListWidgetItem();

	// default state is normal
	thisButton->setIcon(buttonForState(1));

	thisButton->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

// returns icon depending on state
QIcon ButtonInfo::buttonForState(int state)
{
	string bname = buttonName + "_" + to_string(state) + ".png";

	QFileInfo checkFile(QString(bname.c_str()));

	// check if file exists and if yes: Is it really a file and no directory?
	if (checkFile.exists() && checkFile.isFile()) {
		return QIcon(QString(bname.c_str()));
	}

	return QIcon();
}


// GQTButtonsWidget constructor
GQTButtonsWidget::GQTButtonsWidget(double h, double v, vector<string> bicons, QWidget *parent) : QWidget(parent)
{
	static int distanceToMargin = 10;

	buttons.clear();
	for(auto &b : bicons) {
		buttons.push_back(new ButtonInfo(b));
	}

	buttonsWidget = new QListWidget;
	buttonsWidget->setViewMode(QListView::IconMode);
	buttonsWidget->setIconSize(QSize(h, v));
	buttonsWidget->setMovement(QListView::Static);
	//buttonsWidget->setMouseTracking(1);

	for(auto &b : buttons) {
		buttonsWidget->addItem(b->thisButton);
	}

	// maybe call from mother
	buttonsWidget->setCurrentRow(1);
	connect(buttonsWidget,	SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(buttonWasPressed(QListWidgetItem *)) );

	QVBoxLayout *layout = new QVBoxLayout;
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

	setFixedSize(h+distanceToMargin, (v+distanceToMargin)*(buttons.size()));
}



void GQTButtonsWidget::buttonWasPressed(QListWidgetItem* item)
{
	for(int i=0; i<buttonsWidget->count(); i++) { buttonsWidget->item(i)->setIcon(buttons[i]->buttonForState(1)); }

	// starts at 0
	int index = buttonsWidget->currentRow();

	item->setIcon(buttons[index]->buttonForState(2));
}
