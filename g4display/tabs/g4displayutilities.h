#pragma once

/**
 * \file g4displayutilities.h
 * \brief Declaration of the \ref G4DisplayUtilities widget.
 *
 * This header declares a Qt widget used as an auxiliary “Utilities” tab in \ref G4Display.
 * The intent is to provide display-related helpers that do not belong in the main view-control tab.
 *
 */

#include <QtWidgets>      // Qt widgets (QWidget, layouts, etc.)
#include "g4Text.h"
#include "glogger.h"      // Glogger (module logger)
#include "goptions.h"
#include "G4UImanager.hh" // Geant4 UI manager

/**
 * \class G4DisplayUtilities
 * \brief “Utilities” tab for \ref G4Display.
 *
 * This widget is placed in the “Utilities” tab of \ref G4Display. It hosts scene decoration
 * and text controls that redraw the persistent visualization scene when applied.
 *
 * Lifetime and ownership:
 * - Child widgets created with \c this as parent are owned by Qt and deleted automatically.
 * - Logging uses the shared Glogger instance passed by the parent \ref G4Display.
 */
class G4DisplayUtilities : public QWidget
{
	Q_OBJECT

public:
	/**
	 * \brief Construct the utilities tab widget.
	 *
	 * The constructor stores a shared logger and builds the tab contents.
	 *
	 * \param gopt   Shared options object used by the module (may be unused in early implementations).
	 * \param logger Shared logger instance (created/owned upstream, typically by \ref G4Display).
	 * \param parent Optional Qt parent widget for ownership management.
	 */
	G4DisplayUtilities(const std::shared_ptr<GOptions>& gopt,
	                   std::shared_ptr<GLogger>         logger,
	                   QWidget*                         parent = nullptr);

	/**
	 * \brief Destructor.
	 *
	 * Emits a debug-level log message. Child widgets are deleted by Qt parent-child ownership.
	 */
	~G4DisplayUtilities() override { log->debug(DESTRUCTOR, "G4DisplayUtilities"); }

private:
	/**
	 * \brief Shared logger for this widget.
	 *
	 * The logger is expected to remain valid for the lifetime of this widget.
	 */
	std::shared_ptr<GLogger> log;
	std::shared_ptr<GOptions> gopts;
	std::vector<g4display::G4SceneText> sceneTexts;

	QCheckBox* scaleCheck = nullptr;
	QCheckBox* axesCheck = nullptr;
	QCheckBox* eventIDCheck = nullptr;
	QCheckBox* dateCheck = nullptr;
	QCheckBox* logo2DCheck = nullptr;
	QCheckBox* logo3DCheck = nullptr;
	QCheckBox* frameCheck = nullptr;
	QComboBox* frameColorCombo = nullptr;
	QDoubleSpinBox* frameLineWidthSpin = nullptr;

	QComboBox* textKindCombo = nullptr;
	QComboBox* textColorCombo = nullptr;
	QComboBox* textLayoutCombo = nullptr;
	QLineEdit* textEdit = nullptr;
	QDoubleSpinBox* textXSpin = nullptr;
	QDoubleSpinBox* textYSpin = nullptr;
	QDoubleSpinBox* textZSpin = nullptr;
	QSpinBox* textSizeSpin = nullptr;
	QDoubleSpinBox* textDxSpin = nullptr;
	QDoubleSpinBox* textDySpin = nullptr;
	QLineEdit* textUnitEdit = nullptr;

private slots:
	void applyDecorations();
	void addText();
	void clearTexts();

signals:
	void sceneOptionsChanged();

private:
	void syncOptionsFromControls();
};
