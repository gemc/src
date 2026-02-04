#include "gQtButtonsWidget.h"
#include <QFileInfo>
#include <QBoxLayout>
#include <QListView>

/* --- ButtonInfo Implementation ---
 *
 * This implementation provides:
 * - construction of a QListWidgetItem configured as selectable/enabled
 * - icon lookup using the "<base>_<state>.svg" filename convention
 */

ButtonInfo::ButtonInfo(const std::string& icon)
	: buttonName(icon) {
	thisButton = new QListWidgetItem();

	// Initialize the item in the "normal" state.
	thisButton->setIcon(iconForState(1));

	// The list item must be enabled/selectable to behave as a clickable icon entry.
	thisButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QIcon ButtonInfo::iconForState(int state) const {
	// Build the expected resource/file name for the requested state.
	std::string filename = buttonName + "_" + std::to_string(state) + ".svg";

	// QFileInfo is used as an existence check; if missing, return an empty icon.
	QFileInfo fileInfo(QString::fromStdString(filename));
	if (fileInfo.exists() && fileInfo.isFile()) {
		return QIcon(QString::fromStdString(filename));
	}
	return QIcon();
}

/* --- GQTButtonsWidget Implementation ---
 *
 * This widget:
 * - builds one ButtonInfo per base icon name
 * - displays them in a QListWidget configured for icon mode
 * - updates icons on press to reflect "normal" vs "pressed" state
 */

GQTButtonsWidget::GQTButtonsWidget(double                          h, double v,
                                   const std::vector<std::string>& bicons,
                                   bool                            vertical, QWidget* parent)
	: QWidget(parent) {
	constexpr int distanceToMargin = 12;

	// Convert icon base names into internal ButtonInfo entries.
	for (const auto& b : bicons) {
		buttons.push_back(new ButtonInfo(b));
	}

	// Create and configure the QListWidget backend (icon mode, fixed icon size).
	buttonsWidget = new QListWidget(this);
	buttonsWidget->setViewMode(QListView::IconMode);
	buttonsWidget->setIconSize(QSize(static_cast<int>(h), static_cast<int>(v)));

	// Remove selection highlight to keep the visual "button" look consistent.
	buttonsWidget->setStyleSheet(
		"background-color: transparent; "
		"QListWidget::item:selected { background: transparent; border: none; }");

	// Insert each QListWidgetItem into the list widget.
	for (auto& b : buttons) {
		buttonsWidget->addItem(b->thisButton);
	}

	// When an item is pressed, update all icons so only the pressed one shows state 2.
	connect(buttonsWidget, &QListWidget::itemPressed,
	        this, &GQTButtonsWidget::buttonWasPressed);

	// Choose layout based on the requested orientation.
	QBoxLayout* layout = vertical
		                     ? static_cast<QBoxLayout*>(new QVBoxLayout(this))
		                     : static_cast<QBoxLayout*>(new QHBoxLayout(this));
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(buttonsWidget);
	setLayout(layout);

	// Size the widget so the icon strip fits exactly (plus a margin factor).
	double hsize = (h + distanceToMargin) * (buttons.size());
	double vsize = v + distanceToMargin;
	if (vertical) {
		hsize = h + distanceToMargin;
		vsize = (v + distanceToMargin) * (buttons.size());
	}
	buttonsWidget->setFixedSize(static_cast<int>(hsize), static_cast<int>(vsize));

	// Final style: remove focus rectangles and keep a transparent background.
	buttonsWidget->setFocusPolicy(Qt::NoFocus);
	buttonsWidget->setStyleSheet(
		"QListWidget { background-color: transparent; }"
		"QListWidget::item { background: transparent; border: none; }"
		"QListWidget::item:selected { background: transparent; border: none; outline: none; }"
	);
}

void GQTButtonsWidget::press_button(int i) {
	// Select the requested row and switch its icon to the "pressed" state.
	buttonsWidget->setCurrentRow(i);
	if (i >= 0 && i < static_cast<int>(buttons.size()))
		buttonsWidget->item(i)->setIcon(buttons[i]->iconForState(2));
}

void GQTButtonsWidget::buttonWasPressed(QListWidgetItem* item) {
	// Reset all items to normal state.
	for (int i = 0; i < buttonsWidget->count(); i++) {
		buttonsWidget->item(i)->setIcon(buttons[i]->iconForState(1));
	}

	// Set the icon for the pressed item.
	int index = buttonsWidget->row(item);
	item->setIcon(buttons[index]->iconForState(2));
}

void GQTButtonsWidget::reset_buttons() {
	// Restore every button icon to the "normal" state (state 1).
	for (auto& b : buttons) {
		b->thisButton->setIcon(b->iconForState(1));
	}
}

/* --- GQTToggleButtonWidget Implementation ---
 *
 * This widget:
 * - creates one checkable QPushButton per title
 * - arranges them vertically/horizontally
 * - tracks the last clicked button index and emits a signal on changes
 */

GQTToggleButtonWidget::GQTToggleButtonWidget(int buttonWidth, int buttonHeight, int borderRadius,
                                             const std::vector<std::string>& titles,
                                             bool vertical, QWidget* parent)
	: QWidget(parent),
	  buttonPressedIndex(-1) {
	QBoxLayout* layout = vertical
		                     ? static_cast<QBoxLayout*>(new QVBoxLayout(this))
		                     : static_cast<QBoxLayout*>(new QHBoxLayout(this));

	// Convert std::string titles to QString for QPushButton labels.
	QStringList buttonStrings;
	for (const auto& title : titles) {
		buttonStrings.append(QString::fromStdString(title));
	}

	// Create a toggleable button for each title.
	for (int i = 0; i < buttonStrings.size(); ++i) {
		QPushButton* button = new QPushButton(buttonStrings[i], this);
		button->setCheckable(true);
		button->setFixedSize(buttonWidth, buttonHeight);

		// Style: rounded corners, bold text, and different background when checked.
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

		// Use a lambda to bind each button click to its index.
		connect(button, &QPushButton::clicked, this, [this, i]() {
			this->setButtonPressed(i);
		});
	}
	setLayout(layout);
}

bool GQTToggleButtonWidget::lastButtonState() const {
	// A missing or invalid index is treated as "not checked".
	if (buttonPressedIndex >= 0 && buttonPressedIndex < buttons.size())
		return buttons[buttonPressedIndex]->isChecked();
	return false;
}

void GQTToggleButtonWidget::toggleButton(int index) {
	// Toggle changes checked->unchecked or unchecked->checked.
	if (index >= 0 && index < buttons.size())
		buttons[index]->toggle();
}

bool GQTToggleButtonWidget::buttonStatus(int index) const {
	// Query individual button state; invalid index returns false.
	if (index >= 0 && index < buttons.size())
		return buttons[index]->isChecked();
	return false;
}

void GQTToggleButtonWidget::setButtonPressed(int index) {
	// Track the last pressed index and notify listeners.
	buttonPressedIndex = index;
	emit buttonPressedIndexChanged(buttonPressedIndex);
}

void GQTToggleButtonWidget::reset_buttons() {
	// Uncheck all buttons; does not change the stored pressed index.
	for (auto& b : buttons) {
		b->setChecked(false);
	}
}
