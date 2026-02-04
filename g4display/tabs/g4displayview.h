#pragma once

/**
 * \file g4displayview.h
 * \brief Declaration of the \ref G4DisplayView tab widget.
 *
 * \ref G4DisplayView provides the main interactive visualization controls exposed by \ref G4Display.
 * It builds Qt widgets (sliders, dropdowns, toggle buttons) that translate user interaction into Geant4
 * visualization commands sent via \c G4UImanager.
 *
 */

// Qt
#include <QtWidgets>

// gemc
#include "glogger.h"
#include "gQtButtonsWidget.h" // Custom toggle button widget

/**
 * \class G4DisplayView
 * \brief “View” tab for \ref G4Display.
 *
 * Responsibilities:
 * - Camera direction control (theta/phi) using sliders and preset dropdowns.
 * - Light direction control (theta/phi) using sliders and preset dropdowns.
 * - View properties such as projection mode and circle segmentation precision.
 * - Scene properties such as culling mode and background color.
 * - Cutaway plane (“slice”) controls for X/Y/Z planes, including union/intersection mode.
 * - Toggleable visualization features (hidden edges, anti-aliasing, auxiliary edges, field lines, axes, scale).
 *
 * Implementation notes:
 * - User actions are translated into Geant4 visualization commands and sent to \c G4UImanager.
 * - This widget relies on Qt parent-child ownership for memory management of child widgets.
 * - Logging uses the shared Glogger instance provided by \ref G4Display.
 */
class G4DisplayView : public QWidget
{
	Q_OBJECT

public:
	/**
	 * \brief Construct the view-control tab.
	 *
	 * The constructor reads initial camera settings from GOptions (via g4display option helpers),
	 * creates all UI controls, arranges them in layouts, and connects Qt signals to slots that
	 * issue commands through \c G4UImanager.
	 *
	 * \param gopts  Shared options object used to initialize defaults (camera angles, etc.).
	 * \param logger Shared logger instance (created/owned upstream, typically by \ref G4Display).
	 * \param parent Optional Qt parent widget for ownership management.
	 */
	G4DisplayView(const std::shared_ptr<GOptions>& gopts,
	              std::shared_ptr<GLogger>         logger,
	              QWidget*                         parent = nullptr);

	/**
	 * \brief Destructor.
	 *
	 * Emits a debug-level log message. Child widgets are deleted by Qt parent-child ownership.
	 */
	~G4DisplayView() override { log->debug(DESTRUCTOR, "G4DisplayView"); }

private:
	// --- Member widgets (owned by Qt via parenting) ---

	/// Input field for the number of points used in magnetic field line visualization.
	QLineEdit* field_npoints = nullptr;

	/// Toggle button group for view/scene flags (hidden lines, AA, auxiliary edges, field lines, axes, scale).
	GQTToggleButtonWidget* buttons_set1 = nullptr;

	/// Camera direction sliders (theta, phi), expressed in degrees.
	QSlider* cameraTheta = nullptr;
	QSlider* cameraPhi   = nullptr;

	/// Preset dropdowns for camera angles (degrees).
	QComboBox* thetaDropdown = nullptr;
	QComboBox* phiDropdown   = nullptr;

	/// Preset dropdowns for light angles (degrees).
	QComboBox* lthetaDropdown = nullptr;
	QComboBox* lphiDropdown   = nullptr;

	/// Dropdowns for view/scene properties.
	QComboBox* perspectiveDropdown     = nullptr;
	QComboBox* precisionDropdown       = nullptr;
	QComboBox* cullingDropdown         = nullptr;
	QComboBox* backgroundColorDropdown = nullptr;

	/// Light direction sliders (theta, phi), expressed in degrees.
	QSlider* lightTheta = nullptr;
	QSlider* lightPhi   = nullptr;

	/// Slice position input fields (values interpreted as millimeters in issued Geant4 commands).
	QLineEdit* sliceXEdit = nullptr;
	QLineEdit* sliceYEdit = nullptr;
	QLineEdit* sliceZEdit = nullptr;

	/// Slice activation checkboxes (enable/disable each plane).
	QCheckBox* sliceXActi = nullptr;
	QCheckBox* sliceYActi = nullptr;
	QCheckBox* sliceZActi = nullptr;

	/// Slice inversion checkboxes (flip plane normal direction).
	QCheckBox* sliceXInve = nullptr;
	QCheckBox* sliceYInve = nullptr;
	QCheckBox* sliceZInve = nullptr;

	/// Slice mode selection: intersection vs union.
	QRadioButton* sliceSectn = nullptr;
	QRadioButton* sliceUnion = nullptr;

	// --- Member data ---

	/// Number of sample points used by the Geant4 magnetic field visualization model.
	int field_NPOINTS = 5;

	/**
	 * \brief Shared logger instance.
	 *
	 * This logger is provided by the parent module and is expected to outlive this widget.
	 */
	std::shared_ptr<GLogger> log;

private slots:
	/**
	 * \brief Update the Geant4 camera viewpoint from the camera sliders.
	 *
	 * Reads \c cameraTheta and \c cameraPhi and sends:
	 * \code
	 * /vis/viewer/set/viewpointThetaPhi <theta> <phi>
	 * \endcode
	 * through \c G4UImanager.
	 */
	void changeCameraDirection();

	/**
	 * \brief Update camera direction from the preset dropdowns.
	 *
	 * The dropdown values are read and issued as a \c /vis/viewer/set/viewpointThetaPhi command.
	 * After issuing the command, the corresponding slider is updated so the UI stays consistent.
	 *
	 * \param which Selects which slider should be updated to match the dropdown:
	 * - 0 : theta updated
	 * - 1 : phi updated
	 */
	void setCameraDirection(int which);

	/**
	 * \brief Update the Geant4 light direction from the light sliders.
	 *
	 * Reads \c lightTheta and \c lightPhi and sends:
	 * \code
	 * /vis/viewer/set/lightsThetaPhi <theta> <phi>
	 * \endcode
	 * through \c G4UImanager.
	 */
	void changeLightDirection();

	/**
	 * \brief Update light direction from the preset dropdowns.
	 *
	 * Issues the \c /vis/viewer/set/lightsThetaPhi command and then updates the corresponding slider.
	 *
	 * \param which Selects which slider should be updated to match the dropdown:
	 * - 0 : theta updated
	 * - 1 : phi updated
	 */
	void setLightDirection(int which);

	/**
	 * \brief Set viewer projection mode (orthogonal or perspective).
	 *
	 * Converts the current dropdown selection into:
	 * \code
	 * /vis/viewer/set/projection <o|p> <angle>
	 * \endcode
	 * where \c o is orthogonal and \c p is perspective with a given angle.
	 */
	void set_projection();

	/**
	 * \brief Set circle segmentation precision.
	 *
	 * Issues:
	 * \code
	 * /vis/viewer/set/lineSegmentsPerCircle <N>
	 * /vis/viewer/flush
	 * \endcode
	 * to update viewer resolution.
	 */
	void set_precision();

	/**
	 * \brief Configure culling behavior.
	 *
	 * Based on the dropdown selection, issues Geant4 commands that enable/disable global culling,
	 * covered-daughters culling, or density-based culling thresholds.
	 */
	void set_culling();

	/**
	 * \brief Set viewer background color.
	 *
	 * Translates a CSS-like color name selection into normalized RGB values and issues:
	 * \code
	 * /vis/viewer/set/background <r> <g> <b>
	 * \endcode
	 */
	void set_background();

	/**
	 * \brief Apply slice (cutaway plane) settings.
	 *
	 * Clears existing cutaway planes, sets cutaway mode (intersection/union), and adds planes for
	 * each enabled axis. Positions are taken from the line edits; inversion flags flip the normal.
	 */
	void slice();

	/**
	 * \brief Clear all slice planes and reset UI state.
	 *
	 * Issues \c /vis/viewer/clearCutawayPlanes and resets activation checkboxes as appropriate.
	 */
	void clearSlices();

	/**
	 * \brief Apply a toggle-button action.
	 *
	 * Uses the toggled button index and state to issue the corresponding visualization change:
	 * - Hidden lines, auxiliary edges, and field lines are controlled via Geant4 commands.
	 * - Anti-aliasing is currently handled via direct OpenGL state changes (viewer-dependent).
	 *
	 * \param index Index of the toggled button within \c buttons_set1.
	 */
	void apply_buttons_set1(int index);

	/**
	 * \brief Update magnetic field line sampling precision.
	 *
	 * Reads \c field_npoints, updates \c field_NPOINTS, and if field lines are active reissues
	 * the magnetic field model commands so the new point count takes effect.
	 */
	void field_precision_changed();
};
