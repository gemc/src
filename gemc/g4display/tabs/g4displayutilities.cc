// g4displayutilities.cc
//
// Implementation of the G4DisplayUtilities tab.
// Doxygen documentation for this class is authoritative in g4displayutilities.h (see rule 7).

#include "g4displayutilities.h"
#include "g4display_options.h"

// c++
#include <sstream>
#include <string>
using namespace std;

namespace {
void addItemsWithCurrent(QComboBox* combo, const QStringList& items, const QString& current) {
	combo->addItems(items);
	if (!current.isEmpty() && combo->findText(current) < 0) {
		combo->addItem(current);
	}
	combo->setCurrentText(current);
}

// Parse a Geant4 colour string ("r g b" floats or a named CSS colour) into a QColor.
QColor qcolorFromG4(const std::string& s) {
	std::istringstream iss(s);
	double r, g, b;
	if (iss >> r >> g >> b) return QColor::fromRgbF(r, g, b);
	QColor c(QString::fromStdString(s));
	return c.isValid() ? c : Qt::white;
}

// Build an 18×18 rounded color swatch icon.
QIcon colorSwatchIcon(const QColor& color) {
	QPixmap pixmap(18, 18);
	pixmap.fill(Qt::transparent);
	QPainter painter(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setBrush(color);
	painter.setPen(QColor(120, 120, 120));
	painter.drawRoundedRect(pixmap.rect().adjusted(1, 1, -2, -2), 3, 3);
	return QIcon(pixmap);
}

QCheckBox* decorationCheckBox(const QString& text, bool checked, QWidget* parent) {
	auto* checkBox = new QCheckBox(text, parent);
	checkBox->setChecked(checked);
	return checkBox;
}

QDoubleSpinBox* doubleSpin(double value, double min, double max, double step, QWidget* parent) {
	auto* spin = new QDoubleSpinBox(parent);
	spin->setRange(min, max);
	spin->setSingleStep(step);
	spin->setValue(value);
	return spin;
}

string yamlString(const string& value) {
	string escaped;
	for (char c : value) {
		if (c == '\\' || c == '"') { escaped.push_back('\\'); }
		escaped.push_back(c);
	}
	return "\"" + escaped + "\"";
}
}

G4DisplayUtilities::G4DisplayUtilities(const std::shared_ptr<GOptions>& gopt,
                                       std::shared_ptr<GLogger>         logger,
                                       QWidget*                         parent)
	: QWidget(parent), log(logger), gopts(gopt) {
	log->debug(CONSTRUCTOR, "G4DisplayUtilities");

	const auto decorations = g4display::getG4Decorations(gopts);
	sceneTexts = g4display::getSceneTexts(gopts);

	scaleCheck   = decorationCheckBox(tr("Scale"), decorations.scale, this);
	axesCheck    = decorationCheckBox(tr("Axes"), decorations.axes, this);
	eventIDCheck = decorationCheckBox(tr("Event ID"), decorations.eventID, this);
	dateCheck    = decorationCheckBox(tr("Date"), decorations.date, this);
	logo2DCheck  = decorationCheckBox(tr("Logo 2D"), decorations.logo2D, this);
	logo3DCheck  = decorationCheckBox(tr("Logo 3D"), decorations.logo, this);
	frameCheck   = decorationCheckBox(tr("Frame"), decorations.frame, this);

	scaleLengthSpin = doubleSpin(decorations.scaleLength, 0.0, 100000.0, 1.0, this);
	scaleUnitEdit = new QLineEdit(QString::fromStdString(decorations.scaleUnit), this);
	scaleDirectionCombo = new QComboBox(this);
	addItemsWithCurrent(scaleDirectionCombo, {"x", "y", "z"},
	                    QString::fromStdString(decorations.scaleDirection));

	frameLineWidthSpin = doubleSpin(decorations.frameLineWidth, 0.1, 20.0, 0.5, this);

	// Initialise colour members from stored options.
	scaleColor = qcolorFromG4(decorations.scaleColor);
	frameColor = qcolorFromG4(decorations.frameColor);
	textColor  = Qt::black;

	// EventID and date font sizes.
	const QString spinBorderStyle = "QSpinBox { border: 1px solid #888; border-radius: 3px; padding: 1px 3px; }";

	eventIDSizeSpin = new QSpinBox(this);
	eventIDSizeSpin->setRange(8, 100);
	eventIDSizeSpin->setValue(decorations.eventIDSize);
	eventIDSizeSpin->setToolTip(tr("Event ID font size"));
	eventIDSizeSpin->setStyleSheet(spinBorderStyle);

	dateSizeSpin = new QSpinBox(this);
	dateSizeSpin->setRange(8, 100);
	dateSizeSpin->setValue(decorations.dateSize);
	dateSizeSpin->setToolTip(tr("Date font size"));
	dateSizeSpin->setStyleSheet(spinBorderStyle);

	// Helper: build a small color-swatch button that opens a color dialog on click.
	auto makeColorBtn = [this](QColor& colorRef, const QString& title) {
		auto* btn = new QToolButton(this);
		btn->setToolTip(title);
		btn->setAutoRaise(true);
		btn->setIconSize(QSize(18, 18));
		btn->setIcon(colorSwatchIcon(colorRef));
		connect(btn, &QToolButton::clicked, this, [this, btn, &colorRef, title]() {
			QColor c = QColorDialog::getColor(colorRef, this, title);
			if (c.isValid()) {
				colorRef = c;
				btn->setIcon(colorSwatchIcon(c));
			}
		});
		return btn;
	};

	auto* scaleColorBtn = makeColorBtn(scaleColor, tr("Scale color"));
	auto* frameColorBtn = makeColorBtn(frameColor, tr("Frame color"));
	textColorBtn = makeColorBtn(textColor, tr("Text color"));

	// Helper: pair a checkbox and a small widget with a small gap in a single container.
	auto makeTightPair = [this](QWidget* left, QWidget* right) {
		auto* container = new QWidget(this);
		auto* hbox = new QHBoxLayout(container);
		hbox->setContentsMargins(0, 0, 0, 0);
		hbox->setSpacing(8);
		hbox->addWidget(left);
		hbox->addWidget(right);
		hbox->addStretch();
		return container;
	};

	// Decoration grid.
	auto* decorationGrid = new QGridLayout;
	// Row 0: Scale (checkbox + color swatch tightly paired) | Axes
	decorationGrid->addWidget(makeTightPair(scaleCheck, scaleColorBtn), 0, 0, 1, 2);
	decorationGrid->addWidget(axesCheck,                                 0, 2);
	// Row 1: Event ID (checkbox + size spinbox) | Date (checkbox + size spinbox)
	decorationGrid->addWidget(makeTightPair(eventIDCheck, eventIDSizeSpin), 1, 0, 1, 2);
	decorationGrid->addWidget(makeTightPair(dateCheck, dateSizeSpin),       1, 2, 1, 2);
	// Row 2: Logo 2D | Logo 3D
	decorationGrid->addWidget(logo2DCheck, 2, 0);
	decorationGrid->addWidget(logo3DCheck, 2, 2);
	// Row 3: Frame (checkbox + color swatch tightly paired)
	decorationGrid->addWidget(makeTightPair(frameCheck, frameColorBtn), 3, 0, 1, 2);
	decorationGrid->addWidget(new QLabel(tr("Scale Length:")),    4, 0);
	decorationGrid->addWidget(scaleLengthSpin,                    4, 1, 1, 2);
	decorationGrid->addWidget(new QLabel(tr("Scale Unit:")),      5, 0);
	decorationGrid->addWidget(scaleUnitEdit,                      5, 1, 1, 2);
	decorationGrid->addWidget(new QLabel(tr("Scale Direction:")), 6, 0);
	decorationGrid->addWidget(scaleDirectionCombo,                6, 1, 1, 2);
	decorationGrid->addWidget(new QLabel(tr("Frame Line Width:")),7, 0);
	decorationGrid->addWidget(frameLineWidthSpin,                 7, 1, 1, 2);

	auto* applyDecorationsButton = new QPushButton(tr("Apply Decorations"), this);
	connect(applyDecorationsButton, &QPushButton::clicked, this, &G4DisplayUtilities::applyDecorations);

	auto* decorationLayout = new QVBoxLayout;
	decorationLayout->addLayout(decorationGrid);
	decorationLayout->addWidget(applyDecorationsButton);

	auto* decorationGroup = new QGroupBox(tr("Scene Decorations"), this);
	decorationGroup->setLayout(decorationLayout);

	textKindCombo = new QComboBox(this);
	textKindCombo->addItems({"2D", "3D"});

	textLayoutCombo = new QComboBox(this);
	textLayoutCombo->addItems({"", "left", "centre", "center", "right"});

	textEdit = new QLineEdit(this);
	textEdit->setPlaceholderText(tr("Text"));

	textXSpin = doubleSpin(0.9, -100000.0, 100000.0, 0.1, this);
	textYSpin = doubleSpin(-0.9, -100000.0, 100000.0, 0.1, this);
	textZSpin = doubleSpin(0.0, -100000.0, 100000.0, 0.1, this);
	textSizeSpin = new QSpinBox(this);
	textSizeSpin->setRange(1, 1000);
	textSizeSpin->setValue(24);
	textDxSpin = doubleSpin(4.0, -100000.0, 100000.0, 0.5, this);
	textDySpin = doubleSpin(4.0, -100000.0, 100000.0, 0.5, this);
	textUnitEdit = new QLineEdit(tr("cm"), this);

	textDxSpin->setToolTip(tr("Pixel offset from the projected text anchor."));
	textDySpin->setToolTip(tr("Pixel offset from the projected text anchor."));

	auto* textPositionGrid = new QGridLayout;
	textPositionGrid->addWidget(new QLabel(tr("X:")), 0, 0);
	textPositionGrid->addWidget(textXSpin, 0, 1);
	textPositionGrid->addWidget(new QLabel(tr("Y:")), 1, 0);
	textPositionGrid->addWidget(textYSpin, 1, 1);
	textPositionGrid->addWidget(new QLabel(tr("Z:")), 2, 0);
	textPositionGrid->addWidget(textZSpin, 2, 1);
	textPositionGrid->addWidget(new QLabel(tr("Size:")), 3, 0);
	textPositionGrid->addWidget(textSizeSpin, 3, 1);

	auto* textSettingsGrid = new QGridLayout;
	textSettingsGrid->addWidget(new QLabel(tr("Kind:")), 0, 0);
	textSettingsGrid->addWidget(textKindCombo, 0, 1);
	textSettingsGrid->addWidget(new QLabel(tr("Text:")), 1, 0);
	textSettingsGrid->addWidget(textEdit, 1, 1);
	textSettingsGrid->addWidget(textColorBtn, 1, 2);
	textSettingsGrid->addWidget(new QLabel(tr("Layout:")), 2, 0);
	textSettingsGrid->addWidget(textLayoutCombo, 2, 1);
	textSettingsGrid->addWidget(new QLabel(tr("Unit:")), 3, 0);
	textSettingsGrid->addWidget(textUnitEdit, 3, 1);
	textSettingsGrid->addWidget(new QLabel(tr("dX (px):")), 4, 0);
	textSettingsGrid->addWidget(textDxSpin, 4, 1);
	textSettingsGrid->addWidget(new QLabel(tr("dY (px):")), 5, 0);
	textSettingsGrid->addWidget(textDySpin, 5, 1);

	auto* textGrid = new QHBoxLayout;
	textGrid->addLayout(textPositionGrid);
	textGrid->addLayout(textSettingsGrid);

	auto* addTextButton = new QPushButton(tr("Add Text"), this);
	connect(addTextButton, &QPushButton::clicked, this, &G4DisplayUtilities::addText);

	auto* clearTextButton = new QPushButton(tr("Clear Text"), this);
	connect(clearTextButton, &QPushButton::clicked, this, &G4DisplayUtilities::clearTexts);

	auto* textButtonLayout = new QHBoxLayout;
	textButtonLayout->addWidget(addTextButton);
	textButtonLayout->addWidget(clearTextButton);

	auto* textLayout = new QVBoxLayout;
	textLayout->addLayout(textGrid);
	textLayout->addLayout(textButtonLayout);

	auto* textGroup = new QGroupBox(tr("Scene Text"), this);
	textGroup->setLayout(textLayout);

	auto* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(decorationGroup);
	mainLayout->addWidget(textGroup);
	mainLayout->addStretch();
	setLayout(mainLayout);
}

void G4DisplayUtilities::applyDecorations() {
	syncOptionsFromControls();
	emit sceneOptionsChanged();
}

void G4DisplayUtilities::syncOptionsFromControls() {
	auto colorToRGB = [](const QColor& c) {
		return QString("%1 %2 %3")
		    .arg(c.redF(), 0, 'f', 4)
		    .arg(c.greenF(), 0, 'f', 4)
		    .arg(c.blueF(), 0, 'f', 4)
		    .toStdString();
	};

	ostringstream decorations;
	decorations << "{"
		<< "scale: " << (scaleCheck->isChecked() ? "true" : "false") << ", "
		<< "scaleLength: " << scaleLengthSpin->value() << ", "
		<< "scaleUnit: " << yamlString(scaleUnitEdit->text().toStdString()) << ", "
		<< "scaleDirection: " << yamlString(scaleDirectionCombo->currentText().toStdString()) << ", "
		<< "scaleColor: " << yamlString(colorToRGB(scaleColor)) << ", "
		<< "axes: " << (axesCheck->isChecked() ? "true" : "false") << ", "
		<< "eventID: " << (eventIDCheck->isChecked() ? "true" : "false") << ", "
		<< "eventIDSize: " << eventIDSizeSpin->value() << ", "
		<< "date: " << (dateCheck->isChecked() ? "true" : "false") << ", "
		<< "dateSize: " << dateSizeSpin->value() << ", "
		<< "logo2D: " << (logo2DCheck->isChecked() ? "true" : "false") << ", "
		<< "logo: " << (logo3DCheck->isChecked() ? "true" : "false") << ", "
		<< "frame: " << (frameCheck->isChecked() ? "true" : "false") << ", "
		<< "frameColor: " << yamlString(colorToRGB(frameColor)) << ", "
		<< "frameLineWidth: " << frameLineWidthSpin->value()
		<< "}";
	gopts->setOptionValueFromString("g4decoration", decorations.str());

	ostringstream texts;
	texts << "[";
	for (size_t i = 0; i < sceneTexts.size(); ++i) {
		const auto& text = sceneTexts[i];
		if (i > 0) { texts << ", "; }
		texts << "{"
			<< "kind: " << yamlString(text.kind) << ", "
			<< "text: " << yamlString(text.text) << ", "
			<< "color: " << yamlString(text.color) << ", "
			<< "layout: " << yamlString(text.layout) << ", "
			<< "x: " << text.x << ", "
			<< "y: " << text.y << ", "
			<< "z: " << text.z << ", "
			<< "unit: " << yamlString(text.unit) << ", "
			<< "size: " << text.size << ", "
			<< "dx: " << text.dx << ", "
			<< "dy: " << text.dy
			<< "}";
	}
	texts << "]";
	gopts->setOptionValueFromString("g4text", texts.str());
}

void G4DisplayUtilities::addText() {
	if (textEdit->text().trimmed().isEmpty()) { return; }

	g4display::G4SceneText text;
	text.kind  = textKindCombo->currentText().toStdString();
	text.color = QString("%1 %2 %3")
	    .arg(textColor.redF(), 0, 'f', 4)
	    .arg(textColor.greenF(), 0, 'f', 4)
	    .arg(textColor.blueF(), 0, 'f', 4)
	    .toStdString();
	text.layout = textLayoutCombo->currentText().toStdString();
	text.text   = textEdit->text().toStdString();
	text.x      = textXSpin->value();
	text.y      = textYSpin->value();
	text.z      = textZSpin->value();
	text.size   = textSizeSpin->value();
	text.unit   = textUnitEdit->text().toStdString();
	text.dx     = textDxSpin->value();
	text.dy     = textDySpin->value();

	sceneTexts.emplace_back(text);
	textEdit->clear();
	syncOptionsFromControls();
	emit sceneOptionsChanged();
}

void G4DisplayUtilities::clearTexts() {
	sceneTexts.clear();
	syncOptionsFromControls();
	emit sceneOptionsChanged();
}
