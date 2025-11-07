#include "gQtButtonsWidget.h"
#include <QFileInfo>
#include <QBoxLayout>
#include <QListView>

/* --- ButtonInfo Implementation --- */

/**
 * \brief Constructs a ButtonInfo object.
 *
 * Initializes the buttonName with the provided icon string,
 * creates a new QListWidgetItem, sets its icon for the normal state,
 * and enables selection.
 *
 * \param icon The base icon name.
 */
ButtonInfo::ButtonInfo(const std::string &icon)
		: buttonName(icon) {
	thisButton = new QListWidgetItem();
	// Set the default (normal) state icon.
	thisButton->setIcon(iconForState(1));
	thisButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

/**
 * \brief Returns a QIcon for the given state.
 *
 * Constructs a filename by appending "_" and the state number to the base button name.
 * Checks if the file exists; if so, returns the QIcon constructed from that file,
 * otherwise returns an empty QIcon.
 *
 * \param state The state identifier (e.g., 1 for normal, 2 for pressed).
 * \return QIcon corresponding to the state.
 */
QIcon ButtonInfo::iconForState(int state) const {
	std::string filename = buttonName + "_" + std::to_string(state) + ".svg";
	QFileInfo fileInfo(QString::fromStdString(filename));
	if (fileInfo.exists() && fileInfo.isFile()) {
		return QIcon(QString::fromStdString(filename));
	}
	return QIcon();
}

/* --- GQTButtonsWidget Implementation --- */

/**
 * \brief Constructs a GQTButtonsWidget.
 *
 * Initializes the widget by creating ButtonInfo objects from the provided
 * icon names, setting up a QListWidget in icon mode, applying styling,
 * and arranging the layout based on the specified orientation.
 *
 * \param h Icon width.
 * \param v Icon height.
 * \param bicons Vector of base icon names.
 * \param vertical If true, lays out the buttons vertically; otherwise horizontally.
 * \param parent Pointer to the parent widget.
 */
GQTButtonsWidget::GQTButtonsWidget(double h, double v,
								   const std::vector<std::string> &bicons,
								   bool vertical, QWidget *parent)
		: QWidget(parent) {
	constexpr int distanceToMargin = 12;
	// Create ButtonInfo objects for each icon name.
	for (const auto &b : bicons) {
		buttons.push_back(new ButtonInfo(b));
	}

	// Create and configure the QListWidget for displaying icons.
	buttonsWidget = new QListWidget(this);
	buttonsWidget->setViewMode(QListView::IconMode);
	buttonsWidget->setIconSize(QSize(static_cast<int>(h), static_cast<int>(v)));

	// Remove selection highlight.
	buttonsWidget->setStyleSheet(
			"background-color: transparent; "
			"QListWidget::item:selected { background: transparent; border: none; }");

	// Add each ButtonInfo's QListWidgetItem to the widget.
	for (auto &b : buttons) {
		buttonsWidget->addItem(b->thisButton);
	}

	// Connect the itemPressed signal to the buttonWasPressed slot.
	connect(buttonsWidget, &QListWidget::itemPressed,
			this, &GQTButtonsWidget::buttonWasPressed);

	// Set up layout based on orientation.
	QBoxLayout *layout = vertical ? static_cast<QBoxLayout *>(new QVBoxLayout(this))
								  : static_cast<QBoxLayout *>(new QHBoxLayout(this));
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(buttonsWidget);
	setLayout(layout);

	// Calculate and set a fixed size for the buttonsWidget.
	double hsize = (h + distanceToMargin) * (buttons.size());
	double vsize = v + distanceToMargin;
	if (vertical) {
		hsize = h + distanceToMargin;
		vsize = (v + distanceToMargin) * (buttons.size());
	}
	buttonsWidget->setFixedSize(static_cast<int>(hsize), static_cast<int>(vsize));

	// Additional styling.
	buttonsWidget->setFocusPolicy(Qt::NoFocus);
	buttonsWidget->setStyleSheet(
			"QListWidget { background-color: transparent; }"
			"QListWidget::item { background: transparent; border: none; }"
			"QListWidget::item:selected { background: transparent; border: none; outline: none; }"
	);
}

/**
 * \brief Programmatically presses the button at index i.
 *
 * Sets the current row in the QListWidget and updates the icon to the pressed state.
 *
 * \param i Index of the button to press.
 */
void GQTButtonsWidget::press_button(int i) {
	buttonsWidget->setCurrentRow(i);
	if (i >= 0 && i < static_cast<int>(buttons.size()))
		buttonsWidget->item(i)->setIcon(buttons[i]->iconForState(2));
}

/**
 * \brief Slot called when a button is pressed.
 *
 * Resets all buttons to their normal state and then sets the pressed button's icon
 * to the pressed state.
 *
 * \param item Pointer to the QListWidgetItem that was pressed.
 */
void GQTButtonsWidget::buttonWasPressed(QListWidgetItem *item) {
	// Reset all items to normal state.
	for (int i = 0; i < buttonsWidget->count(); i++) {
		buttonsWidget->item(i)->setIcon(buttons[i]->iconForState(1));
	}
	// Set the icon for the pressed item.
	int index = buttonsWidget->row(item);
	item->setIcon(buttons[index]->iconForState(2));
}


void GQTButtonsWidget::reset_buttons() {
	for (auto &b : buttons) {
		b->thisButton->setIcon(b->iconForState(1));
	}
}


/* --- GQTToggleButtonWidget Implementation --- */

/**
 * \brief Constructs a GQTToggleButtonWidget.
 *
 * Creates toggleable QPushButtons with specified dimensions and styles.
 * The widget layout is chosen based on the vertical parameter. A lambda is used
 * for signal-slot connections to capture each button's index.
 *
 * \param buttonWidth Fixed width of each button.
 * \param buttonHeight Fixed height of each button.
 * \param borderRadius Border radius for the button corners.
 * \param titles Vector of button titles.
 * \param vertical If true, lays out the buttons vertically; otherwise horizontally.
 * \param parent Pointer to the parent widget.
 */
GQTToggleButtonWidget::GQTToggleButtonWidget(int buttonWidth, int buttonHeight, int borderRadius,
											 const std::vector<std::string> &titles,
											 bool vertical, QWidget *parent)
		: QWidget(parent),
		  buttonPressedIndex(-1) {
	QBoxLayout *layout = vertical ? static_cast<QBoxLayout *>(new QVBoxLayout(this))
								  : static_cast<QBoxLayout *>(new QHBoxLayout(this));

	// Convert std::string titles to a QStringList.
	QStringList buttonStrings;
	for (const auto &title : titles) {
		buttonStrings.append(QString::fromStdString(title));
	}

	// Create a toggleable button for each title.
	for (int i = 0; i < buttonStrings.size(); ++i) {
		QPushButton *button = new QPushButton(buttonStrings[i], this);
		button->setCheckable(true);
		button->setFixedSize(buttonWidth, buttonHeight);

		// Set stylesheet for rounded corners and dynamic background color.
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

		// Connect the clicked signal with a lambda capturing the index.
		connect(button, &QPushButton::clicked, this, [this, i]() {
			this->setButtonPressed(i);
		});
	}
	setLayout(layout);
}

/**
 * \brief Returns the state of the last pressed button.
 *
 * \return True if the last pressed button is checked; false otherwise.
 */
bool GQTToggleButtonWidget::lastButtonState() const {
	if (buttonPressedIndex >= 0 && buttonPressedIndex < buttons.size())
		return buttons[buttonPressedIndex]->isChecked();
	return false;
}

/**
 * \brief Toggles the button at the given index.
 *
 * \param index The index of the button to toggle.
 */
void GQTToggleButtonWidget::toggleButton(int index) {
	if (index >= 0 && index < buttons.size())
		buttons[index]->toggle();
}

/**
 * \brief Returns the status of the button at the given index.
 *
 * \param index The index of the button.
 * \return True if the button is checked; false otherwise.
 */
bool GQTToggleButtonWidget::buttonStatus(int index) const {
	if (index >= 0 && index < buttons.size())
		return buttons[index]->isChecked();
	return false;
}

/**
 * \brief Slot to set the currently pressed button.
 *
 * Updates the internal index of the pressed button and emits a signal.
 *
 * \param index The index of the button that was pressed.
 */
void GQTToggleButtonWidget::setButtonPressed(int index) {
	buttonPressedIndex = index;
	emit buttonPressedIndexChanged(buttonPressedIndex);
}

void GQTToggleButtonWidget::reset_buttons() {
	for (auto &b : buttons) {
		b->setChecked(false);
	}
}
