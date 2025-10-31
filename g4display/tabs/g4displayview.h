#pragma once

// Qt
#include <QtWidgets> // Includes necessary Qt Widget headers

// gemc
#include "glogger.h"          // Logging utility
#include "gQtButtonsWidget.h" // Custom toggle button widget
#include "goptions.h"         // Forward GOptions type


// Geant4
#include "G4UImanager.hh" // Geant4 UI manager access


/**
 * @class G4DisplayView
 * @brief A QWidget tab providing controls for Geant4 camera, lighting, slicing, and view styles.
 *
 * This widget allows users to interactively change visualization parameters like
 * viewpoint angles, light source direction, apply cutaway planes (slices) to the
 * geometry, and toggle rendering options like hidden lines, anti-aliasing,
 * auxiliary edges, and magnetic field lines. It communicates these changes
 * to Geant4 via the G4UImanager.
 * It is typically added as a tab to the main G4Display widget.
 */
class G4DisplayView : public QWidget {
	Q_OBJECT // Macro required for signals, slots, and Qt meta-object features

public:
    /**
     * @brief Constructs the G4DisplayView widget.
     * @param gopts Pointer to the GOptions object for initial settings (e.g., camera angles).
     * @param logger Pointer to the GLogger instance for logging messages. Must not be null. Passed from parent G4Display.
     * @param parent Optional parent QWidget for Qt's memory management.
     */
	G4DisplayView(const std::shared_ptr<GOptions>& gopts,	std::shared_ptr<GLogger> logger, QWidget* parent = nullptr);

    /**
     * @brief Destructor. Logs the destruction event using the provided logger.
     *        Qt's parent-child mechanism handles the deletion of child widgets
     *        (like QLineEdit, QSlider, etc.) that were created with `this` as their parent.
     */
	~G4DisplayView() override { log->debug(DESTRUCTOR, "G4DisplayView"); }

private:
    // --- Member Widgets (Pointers managed by Qt parenting) ---

    /** @brief Input field for setting the number of points used in field line visualization. */
	QLineEdit* field_npoints;

    /** @brief Custom widget containing a group of toggle buttons for view options (Hidden Lines, AA, etc.). */
	GQTToggleButtonWidget* buttons_set1;

    /** @brief Slider controlling the camera's elevation angle (theta). */
	QSlider *cameraTheta;
    /** @brief Slider controlling the camera's azimuthal angle (phi). */
    QSlider *cameraPhi;

	QComboBox *thetaDropdown, *phiDropdown;
	QComboBox *lthetaDropdown, *lphiDropdown;
	QComboBox *perspectiveDropdown, *precisionDropdown;
	QComboBox *cullingDropdown, *backgroundColorDropdown;



    /** @brief Slider controlling the light source's elevation angle (theta). */
	QSlider *lightTheta;
    /** @brief Slider controlling the light source's azimuthal angle (phi). */
    QSlider *lightPhi;

    /** @brief Input field for the X-coordinate position of the slice plane. */
	QLineEdit *sliceXEdit;
    /** @brief Input field for the Y-coordinate position of the slice plane. */
    QLineEdit *sliceYEdit;
    /** @brief Input field for the Z-coordinate position of the slice plane. */
    QLineEdit *sliceZEdit;

    /** @brief Checkbox to activate/deactivate the slice plane along the X-axis. */
	QCheckBox *sliceXActi;
    /** @brief Checkbox to activate/deactivate the slice plane along the Y-axis. */
    QCheckBox *sliceYActi;
    /** @brief Checkbox to activate/deactivate the slice plane along the Z-axis. */
    QCheckBox *sliceZActi;

    /** @brief Checkbox to invert the direction of the cut for the X-axis slice plane. */
	QCheckBox *sliceXInve;
    /** @brief Checkbox to invert the direction of the cut for the Y-axis slice plane. */
    QCheckBox *sliceYInve;
    /** @brief Checkbox to invert the direction of the cut for the Z-axis slice plane. */
    QCheckBox *sliceZInve;

    /** @brief Radio button to select the 'intersection' mode for combining multiple slice planes. */
	QRadioButton *sliceSectn;
    /** @brief Radio button to select the 'union' mode for combining multiple slice planes. */
    QRadioButton *sliceUnion;

    // --- Member Data ---

    /** @brief Stores the current number of points to use for magnetic field line visualization. Default is 5. */
	int field_NPOINTS = 5;

    /**
     * @brief Non-owning pointer to the GLogger instance.
     * This logger is typically created and owned by the parent G4Display widget and passed
     * during construction. It must remain valid for the lifetime of this G4DisplayView object.
     */
	std::shared_ptr<GLogger> log;

private slots:
    /**
     * @brief Slot triggered when either the cameraTheta or cameraPhi slider's value changes.
     * Reads the current values from both sliders and sends the updated viewpoint
     * command (`/vis/viewer/set/viewpointThetaPhi`) to the Geant4 UImanager.
     */
	void changeCameraDirection();


	void setCameraDirection(int which);


    /**
     * @brief Slot triggered when either the lightTheta or lightPhi slider's value changes.
     * Reads the current values from both sliders and sends the updated light source direction
     * command (`/vis/viewer/set/lightsThetaPhi`) to the Geant4 UImanager.
     */
	void changeLightDirection();


	void setLightDirection(int which);




	void set_projection();
	void set_precision();
	void set_culling();
	void set_background();




    /**
     * @brief Slot triggered by changes in any of the slice control widgets (QLineEdit returnPressed,
     * QCheckBox stateChanged, QRadioButton toggled).
     * Reads the current state of all slice controls, sets the cutaway mode (intersection/union),
     * clears any existing cutaway planes, and adds new planes based on the active checkboxes,
     * positions, and inversion flags by sending commands to the Geant4 UImanager.
     */
	void slice();

    /**
     * @brief Slot triggered by the 'Clear Slices' QPushButton.
     * Sends the command (`/vis/viewer/clearCutawayPlanes`) to the Geant4 UImanager
     * to remove all active slice planes. Also resets the state of the activation checkboxes.
     */
	void clearSlices();

    /**
     * @brief Slot triggered when a button within the `buttons_set1` widget is toggled.
     * @param index The integer index of the button within the `GQTToggleButtonWidget` that was toggled.
     *              Expected mapping: 0: Hidden Lines, 1: Anti-Aliasing, 2: Auxiliary Edges, 3: Field Lines.
     * Applies the corresponding Geant4 visualization command based on the button index and its new state
     * (on/off). May involve direct OpenGL calls for Anti-Aliasing.
     */
	void apply_buttons_set1(int index);

    /**
     * @brief Slot triggered when the user presses Enter in the `field_npoints` QLineEdit.
     * Reads the text value, converts it to an integer, validates it, and updates the
     * `field_NPOINTS` member. If the field line visualization is currently active (button index 3),
     * it triggers an update in Geant4 by removing the old field model and adding a new one
     * with the updated number of points.
     */
	void field_precision_changed();
};

