#include "gQtButtonsWidget.h"
#include <QBoxLayout>
#include <QFile>
#include <QListView>
#include <QEvent>
#include <QShowEvent>

/* --- ButtonInfo Implementation ---
 *
 * This implementation provides:
 * - construction of a QListWidgetItem configured as selectable/enabled
 * - icon lookup using the "<base>_<state>.svg" filename convention
 */

ButtonInfo::ButtonInfo(const std::string& icon)
	: buttonName(icon) {
	thisButton = new QListWidgetItem();

	// The list item must be enabled/selectable to behave as a clickable icon entry.
	thisButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

QIcon ButtonInfo::iconForState(int state, const QSize& iconSize, const QPalette& palette) const {
	// Build the expected resource/file name for the requested state.
	const std::string filename = buttonName + "_" + std::to_string(state) + ".svg";

	QFile file(QString::fromStdString(filename));
	if (!file.open(QIODevice::ReadOnly)) {
		return QIcon();
	}

	QString svg = QString::fromUtf8(file.readAll());
	const QColor foreground = state == 2
	                          ? palette.color(QPalette::HighlightedText)
	                          : palette.color(QPalette::WindowText);
	const QColor selectedBackground = palette.color(QPalette::Highlight);

	svg.replace("width=\"48\" height=\"48\"",
	            QString("width=\"%1\" height=\"%2\"").arg(iconSize.width()).arg(iconSize.height()));
	svg.replace("currentColor", foreground.name(QColor::HexRgb));
	svg.replace("#aaddff", selectedBackground.name(QColor::HexRgb), Qt::CaseInsensitive);

	QPixmap pixmap;
	if (!pixmap.loadFromData(svg.toUtf8(), "SVG")) {
		return QIcon();
	}
	return QIcon(pixmap);
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

	// SVG colors are resolved from the Qt palette in ButtonInfo::iconForState().
	buttonsWidget->setStyleSheet(
		"background-color: transparent; "
		"QListWidget::item:selected { background: transparent; border: none; }");

	// Insert each QListWidgetItem into the list widget.
	for (auto& b : buttons) {
		b->thisButton->setIcon(b->iconForState(1, buttonsWidget->iconSize(), buttonsWidget->palette()));
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

	refresh_icons();
}

void GQTButtonsWidget::press_button(int i) {
	// Select the requested row and switch its icon to the "pressed" state.
	buttonsWidget->setCurrentRow(i);
	refresh_icons();
}

void GQTButtonsWidget::buttonWasPressed(QListWidgetItem* item) {
	buttonsWidget->setCurrentItem(item);
	refresh_icons();
}

void GQTButtonsWidget::reset_buttons() {
	// Restore every button icon to the "normal" state (state 1).
	buttonsWidget->setCurrentRow(-1);
	refresh_icons();
}

void GQTButtonsWidget::refresh_icons() {
	if (!buttonsWidget) { return; }

	const int currentRow = buttonsWidget->currentRow();
	const QPalette palette = buttonsWidget->palette();
	const QSize iconSize = buttonsWidget->iconSize();

	for (int row = 0; row < buttonsWidget->count(); row++) {
		const int state = row == currentRow ? 2 : 1;
		buttonsWidget->item(row)->setIcon(buttons[row]->iconForState(state, iconSize, palette));
	}
}

void GQTButtonsWidget::changeEvent(QEvent* event) {
	QWidget::changeEvent(event);

	if (event->type() == QEvent::PaletteChange ||
	    event->type() == QEvent::ApplicationPaletteChange ||
	    event->type() == QEvent::StyleChange) {
		refresh_icons();
	}
}

void GQTButtonsWidget::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);
	refresh_icons();
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

		// SVG handles all visual states (frame, highlight, text color); no CSS decoration needed.
		button->setStyleSheet(
			"QPushButton { border: none; background-color: transparent; padding: 0; margin: 0; }"
			"QPushButton:checked { background-color: transparent; }"
		);

		layout->addWidget(button);
		buttons.push_back(button);

		// Use a lambda to bind each button click to its index.
		connect(button, &QPushButton::clicked, this, [this, i]() {
			this->setButtonPressed(i);
		});
		connect(button, &QPushButton::toggled, this, [this]() {
			refresh_svg_icons();
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

void GQTToggleButtonWidget::setSvgButtonIcon(int index, const QString& svgResourcePath, const QSize& iconSize) {
	if (index < 0 || index >= static_cast<int>(buttons.size())) { return; }

	// Grow the svgIcons table to cover this index.
	while (svgIcons.size() <= index) { svgIcons.append(SvgIcon{}); }

	const QSize sz = iconSize.isValid() ? iconSize
	               : QSize(buttons[index]->width() - 6, buttons[index]->height() - 6);
	svgIcons[index] = SvgIcon{ svgResourcePath, sz };
	refresh_svg_icons();
}

void GQTToggleButtonWidget::refresh_svg_icons() {
	const QColor windowText = palette().color(QPalette::WindowText);
	const QColor hlText     = palette().color(QPalette::HighlightedText);
	const QColor hlBg       = palette().color(QPalette::Highlight);

	for (int i = 0; i < svgIcons.size(); ++i) {
		const SvgIcon& entry = svgIcons[i];
		if (entry.path.isEmpty() || i >= static_cast<int>(buttons.size())) { continue; }

		QFile f(entry.path);
		if (!f.open(QIODevice::ReadOnly)) { continue; }

		const bool    checked = buttons[i]->isChecked();
		const QColor  fg      = checked ? hlText : windowText;
		const QString bg      = checked ? hlBg.name(QColor::HexRgb) : "none";

		QString svg = QString::fromUtf8(f.readAll());
		svg.replace("width=\"48\" height=\"48\"",
		            QString("width=\"%1\" height=\"%2\"").arg(entry.size.width()).arg(entry.size.height()));
		svg.replace("currentColor", fg.name(QColor::HexRgb));
		svg.replace("#aaddff", bg, Qt::CaseInsensitive);

		QPixmap pix;
		if (pix.loadFromData(svg.toUtf8(), "SVG")) {
			buttons[i]->setIcon(QIcon(pix));
			buttons[i]->setIconSize(entry.size);
		}
	}
}

void GQTToggleButtonWidget::changeEvent(QEvent* event) {
	QWidget::changeEvent(event);
	if (event->type() == QEvent::PaletteChange      ||
	    event->type() == QEvent::ApplicationPaletteChange ||
	    event->type() == QEvent::StyleChange) {
		refresh_svg_icons();
	}
}

void GQTToggleButtonWidget::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);
	refresh_svg_icons();
}
