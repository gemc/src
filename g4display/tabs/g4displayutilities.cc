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

	frameColorCombo = new QComboBox(this);
	addItemsWithCurrent(frameColorCombo, {"red", "green", "blue", "white", "black", "yellow"},
	                    QString::fromStdString(decorations.frameColor));

	frameLineWidthSpin = doubleSpin(decorations.frameLineWidth, 0.1, 20.0, 0.5, this);

	auto* decorationGrid = new QGridLayout;
	decorationGrid->addWidget(scaleCheck, 0, 0);
	decorationGrid->addWidget(axesCheck, 0, 1);
	decorationGrid->addWidget(eventIDCheck, 1, 0);
	decorationGrid->addWidget(dateCheck, 1, 1);
	decorationGrid->addWidget(logo2DCheck, 2, 0);
	decorationGrid->addWidget(logo3DCheck, 2, 1);
	decorationGrid->addWidget(frameCheck, 3, 0);
	decorationGrid->addWidget(new QLabel(tr("Frame Color:")), 4, 0);
	decorationGrid->addWidget(frameColorCombo, 4, 1);
	decorationGrid->addWidget(new QLabel(tr("Frame Line Width:")), 5, 0);
	decorationGrid->addWidget(frameLineWidthSpin, 5, 1);

	auto* applyDecorationsButton = new QPushButton(tr("Apply Decorations"), this);
	connect(applyDecorationsButton, &QPushButton::clicked, this, &G4DisplayUtilities::applyDecorations);

	auto* decorationLayout = new QVBoxLayout;
	decorationLayout->addLayout(decorationGrid);
	decorationLayout->addWidget(applyDecorationsButton);

	auto* decorationGroup = new QGroupBox(tr("Scene Decorations"), this);
	decorationGroup->setLayout(decorationLayout);

	textKindCombo = new QComboBox(this);
	textKindCombo->addItems({"2D", "3D"});

	textColorCombo = new QComboBox(this);
	textColorCombo->addItems({"black", "blue", "green", "red", "white", "yellow"});

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

	auto* textGrid = new QGridLayout;
	textGrid->addWidget(new QLabel(tr("Kind:")), 0, 0);
	textGrid->addWidget(textKindCombo, 0, 1);
	textGrid->addWidget(new QLabel(tr("Color:")), 0, 2);
	textGrid->addWidget(textColorCombo, 0, 3);
	textGrid->addWidget(new QLabel(tr("Layout:")), 1, 0);
	textGrid->addWidget(textLayoutCombo, 1, 1);
	textGrid->addWidget(new QLabel(tr("Text:")), 1, 2);
	textGrid->addWidget(textEdit, 1, 3);
	textGrid->addWidget(new QLabel(tr("X:")), 2, 0);
	textGrid->addWidget(textXSpin, 2, 1);
	textGrid->addWidget(new QLabel(tr("Y:")), 2, 2);
	textGrid->addWidget(textYSpin, 2, 3);
	textGrid->addWidget(new QLabel(tr("Z:")), 3, 0);
	textGrid->addWidget(textZSpin, 3, 1);
	textGrid->addWidget(new QLabel(tr("Size:")), 3, 2);
	textGrid->addWidget(textSizeSpin, 3, 3);
	textGrid->addWidget(new QLabel(tr("Unit:")), 4, 0);
	textGrid->addWidget(textUnitEdit, 4, 1);
	textGrid->addWidget(new QLabel(tr("dX:")), 4, 2);
	textGrid->addWidget(textDxSpin, 4, 3);
	textGrid->addWidget(new QLabel(tr("dY:")), 5, 0);
	textGrid->addWidget(textDySpin, 5, 1);

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
	ostringstream decorations;
	decorations << "{"
		<< "scale: " << (scaleCheck->isChecked() ? "true" : "false") << ", "
		<< "axes: " << (axesCheck->isChecked() ? "true" : "false") << ", "
		<< "eventID: " << (eventIDCheck->isChecked() ? "true" : "false") << ", "
		<< "date: " << (dateCheck->isChecked() ? "true" : "false") << ", "
		<< "logo2D: " << (logo2DCheck->isChecked() ? "true" : "false") << ", "
		<< "logo: " << (logo3DCheck->isChecked() ? "true" : "false") << ", "
		<< "frame: " << (frameCheck->isChecked() ? "true" : "false") << ", "
		<< "frameColor: " << yamlString(frameColorCombo->currentText().toStdString()) << ", "
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
	text.kind   = textKindCombo->currentText().toStdString();
	text.color  = textColorCombo->currentText().toStdString();
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
