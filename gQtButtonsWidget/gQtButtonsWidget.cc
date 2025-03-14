#include "gQtButtonsWidget.h"
#include <QFileInfo>
#include <QBoxLayout>
#include <QListView>

// --- ButtonInfo Implementation ---
ButtonInfo::ButtonInfo(const std::string &icon)
		: buttonName(icon)
{
	thisButton = new QListWidgetItem();
	// Set the default (normal) state icon
	thisButton->setIcon(iconForState(1));
	thisButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QIcon ButtonInfo::iconForState(int state) const {
	std::string filename = buttonName + "_" + std::to_string(state) + ".svg";
	QFileInfo fileInfo(QString::fromStdString(filename));
	if (fileInfo.exists() && fileInfo.isFile()) {
		return QIcon(QString::fromStdString(filename));
	}
	return QIcon();
}

// --- GQTButtonsWidget Implementation ---
GQTButtonsWidget::GQTButtonsWidget(double h, double v,
								   const std::vector<std::string> &bicons,
								   bool vertical, QWidget *parent)
		: QWidget(parent)
{
	constexpr int distanceToMargin = 12;
	for (const auto &b : bicons) {
		buttons.push_back(new ButtonInfo(b));
	}

	buttonsWidget = new QListWidget(this);
	buttonsWidget->setViewMode(QListView::IconMode);
	buttonsWidget->setIconSize(QSize(static_cast<int>(h), static_cast<int>(v)));

	// Remove selection highlight (no horizontal line on selection)
	buttonsWidget->setStyleSheet(
			"background-color: transparent; "
			"QListWidget::item:selected { background: transparent; border: none; }");

	// Add buttons to the widget
	for (auto &b : buttons) {
		buttonsWidget->addItem(b->thisButton);
	}

	// Use modern connect syntax
	connect(buttonsWidget, &QListWidget::itemPressed,
			this, &GQTButtonsWidget::buttonWasPressed);

	QBoxLayout *layout = vertical ? static_cast<QBoxLayout*>(new QVBoxLayout(this))
								  : static_cast<QBoxLayout*>(new QHBoxLayout(this));
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(buttonsWidget);
	setLayout(layout);

	// Set fixed size based on orientation
	double hsize = (h + distanceToMargin) * (buttons.size());
	double vsize = v + distanceToMargin;
	if (vertical) {
		hsize = h + distanceToMargin;
		vsize = (v + distanceToMargin) * (buttons.size());
	}
	buttonsWidget->setFixedSize(static_cast<int>(hsize), static_cast<int>(vsize));


	buttonsWidget->setFocusPolicy(Qt::NoFocus);
	buttonsWidget->setStyleSheet(
			"QListWidget { background-color: transparent; }"
			"QListWidget::item { background: transparent; border: none; }"
			"QListWidget::item:selected { background: transparent; border: none; outline: none; }"
	);

}

void GQTButtonsWidget::press_button(int i) {
	buttonsWidget->setCurrentRow(i);
	if (i >= 0 && i < static_cast<int>(buttons.size()))
		buttonsWidget->item(i)->setIcon(buttons[i]->iconForState(2));
}

void GQTButtonsWidget::buttonWasPressed(QListWidgetItem *item) {
	// Reset all items to their normal state
	for (int i = 0; i < buttonsWidget->count(); i++) {
		buttonsWidget->item(i)->setIcon(buttons[i]->iconForState(1));
	}
	// Set the pressed item to the selected state icon
	int index = buttonsWidget->row(item);
	item->setIcon(buttons[index]->iconForState(2));
}

// --- GQTToggleButtonWidget Implementation ---
GQTToggleButtonWidget::GQTToggleButtonWidget(int buttonWidth, int buttonHeight, int borderRadius,
											 const std::vector<std::string> &titles,
											 bool vertical, QWidget *parent)
		: QWidget(parent),
		  buttonPressedIndex(-1)
{
	QBoxLayout *layout = vertical ? static_cast<QBoxLayout*>(new QVBoxLayout(this))
								  : static_cast<QBoxLayout*>(new QHBoxLayout(this));

	// Convert std::string titles to QStringList
	QStringList buttonStrings;
	for (const auto &title : titles) {
		buttonStrings.append(QString::fromStdString(title));
	}

	for (int i = 0; i < buttonStrings.size(); ++i) {
		QPushButton *button = new QPushButton(buttonStrings[i], this);
		button->setCheckable(true);
		button->setFixedSize(buttonWidth, buttonHeight);

		// Apply stylesheet for rounded shape and dynamic background color
		button->setStyleSheet(QString(
				"QPushButton {"
				"    border-radius: %1px;"
				"    border: 2px solid black;"
				"    background-color: rgba(255, 0, 0, 150);"
				"    font-weight: bold;"
				"}"
				"QPushButton:checked {"
				"    background-color: rgba(0, 255, 0, 150);"
				"}"
		).arg(borderRadius));

		layout->addWidget(button);
		buttons.push_back(button);

		// Use a lambda to capture the index instead of QSignalMapper
		connect(button, &QPushButton::clicked, this, [this, i]() {
			this->setButtonPressed(i);
		});
	}
	setLayout(layout);
}

bool GQTToggleButtonWidget::lastButtonState() const {
	if (buttonPressedIndex >= 0 && buttonPressedIndex < buttons.size())
		return buttons[buttonPressedIndex]->isChecked();
	return false;
}

void GQTToggleButtonWidget::toggleButton(int index) {
	if (index >= 0 && index < buttons.size())
		buttons[index]->toggle();
}

bool GQTToggleButtonWidget::buttonStatus(int index) const {
	if (index >= 0 && index < buttons.size())
		return buttons[index]->isChecked();
	return false;
}

void GQTToggleButtonWidget::setButtonPressed(int index) {
	buttonPressedIndex = index;
	emit buttonPressedIndexChanged(buttonPressedIndex);
}
