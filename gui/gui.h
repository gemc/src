#pragma once

// qt
#include <QStackedWidget>   // rightContent
#include <QLineEdit>        // nEvents
#include <QLabel>           // eventNumberLabel
#include <QTimer>           // gtimer
#include <QHBoxLayout>      // createTopButtons

// gemc
#include "gQtButtonsWidget.h"
#include "eventDispenser.h"
#include "gdetectorConstruction.h"
#include "gboard.h"


/**
 * @class GemcGUI
 * @ingroup gemc_gui_module
 *
 * @brief Main GEMC Qt widget that assembles the GUI layout and connects user actions to event processing.
 *
 * GemcGUI builds a three-part interface:
 * - A left-side button bar (GQTButtonsWidget) that selects the active page.
 * - A right-side stacked content area (\c QStackedWidget) hosting the pages.
 * - A top row of run controls (run, cycle, stop, exit) and counters (number of events and event number).
 *
 * Event execution is delegated to an EventDispenser instance provided by the caller. Page widgets are
 * constructed using shared configuration objects (GOptions, GDetectorConstruction, and GBoard) so that
 * the GUI pages remain synchronized with the simulation configuration.
 *
 * @note This class is a \c QWidget and uses Qt signals/slots (Q_OBJECT).
 */
class GemcGUI : public QWidget {

	// metaobject required for non-qt slots
	Q_OBJECT

public:
	/**
	 * @brief Construct the main GUI widget.
	 *
	 * The constructor:
	 * - Creates the left-side button bar.
	 * - Creates a GBoard widget and associates it with a GUI_Session.
	 * - Creates the right-side page content and installs it into a \c QStackedWidget.
	 * - Creates the top run-control buttons and connects signals to the corresponding slots.
	 * - Initializes a \c QTimer used for “Cycle” mode.
	 *
	 * @param gopts Shared options/configuration container used by GUI pages.
	 * @param ed Event execution backend used to run and configure event processing.
	 * @param dc Detector construction object used by setup/tree pages.
	 * @param parent Optional parent widget for Qt ownership.
	 */
	GemcGUI(std::shared_ptr<GOptions>       gopts,
	        std::shared_ptr<EventDispenser> ed,
	        GDetectorConstruction*          dc,
	        QWidget*                        parent = nullptr);

	/**
	 * @brief Destroy the GUI widget and release explicitly owned resources.
	 *
	 * @details The current implementation explicitly deletes some members. When extending the GUI,
	 * keep ownership consistent (Qt parent ownership vs. explicit deletion) for new members.
	 */
	~GemcGUI() override;

private:
	/**
	 * @brief Left-side button bar used to select the active page.
	 *
	 * Ownership/cleanup follows the current implementation which explicitly deletes this pointer
	 * in ~GemcGUI().
	 */
	GQTButtonsWidget* leftButtons;  // left bar buttons

	/**
	 * @brief Right-side stacked widget containing the active GUI pages.
	 *
	 * The active page index is set from the selection in leftButtons. Ownership/cleanup follows the
	 * current implementation which explicitly deletes this pointer in ~GemcGUI().
	 */
	QStackedWidget*   rightContent; // pages controlled by left bar buttons

	/**
	 * @brief Editable field containing the number of events to process when running.
	 *
	 * When edited, the slot \ref GemcGUI::neventsChanged "neventsChanged()" propagates the new value
	 * to EventDispenser::setNumberOfEvents().
	 *
	 * Ownership/cleanup follows the current implementation which explicitly deletes this pointer
	 * in ~GemcGUI().
	 */
	QLineEdit*        nEvents;

	/**
	 * @brief Label displaying the cumulative event number.
	 *
	 * The label is updated after a Run action by \ref GemcGUI::updateGui "updateGui()".
	 *
	 * Ownership/cleanup follows the current implementation which explicitly deletes this pointer
	 * in ~GemcGUI().
	 */
	QLabel*           eventNumberLabel;

	/**
	 * @brief Timer used to implement periodic event processing in “Cycle” mode.
	 *
	 * The timer timeout triggers \ref GemcGUI::cycleBeamOn "cycleBeamOn()".
	 * The timer is created with \c this as parent, so Qt will normally manage its lifetime.
	 */
	QTimer*           gtimer; // for cycling events

	/**
	 * @brief Backend responsible for event processing and run control.
	 *
	 * GemcGUI uses this object to:
	 * - Read the initial total number of events for UI initialization.
	 * - Update the number of events to run when the user edits the field.
	 * - Process events when the user clicks Run or enables Cycle.
	 */
	std::shared_ptr<EventDispenser> eventDispenser;

private:
	/**
	 * @brief Create and initialize the left-side page selection buttons.
	 *
	 * This method instantiates leftButtons and sets up the icon list used for the button bar.
	 */
	void createLeftButtons();

	/**
	 * @brief Create and initialize the right-side stacked content pages.
	 *
	 * This method instantiates rightContent and appends the GUI pages in a fixed order.
	 *
	 * @param gopts Shared options/configuration container forwarded to pages.
	 * @param dc Detector construction forwarded to pages requiring detector context.
	 * @param gb Board widget forwarded to pages that need access to board functionality.
	 */
	void createRightContent(std::shared_ptr<GOptions> gopts,
	                        GDetectorConstruction*    dc,
	                        GBoard*                   gb);

	/**
	 * @brief Create the top control-row widgets and wire their signals to the corresponding slots.
	 *
	 * @param topLayout Layout that receives the top-row widgets (labels, buttons, counters).
	 */
	void createTopButtons(QHBoxLayout* topLayout);

	/**
	 * @brief Update the GUI event counter label after running events.
	 *
	 * The counter increments by the number of events that were run in the last Run action and updates
	 * eventNumberLabel accordingly.
	 */
	void updateGui();

private slots:
	/**
	 * @brief Propagate changes in the “N. Events” field to the backend.
	 *
	 * Reads \ref GemcGUI::nEvents "nEvents" as an integer and calls EventDispenser::setNumberOfEvents()
	 * so subsequent runs use the updated number.
	 */
	void neventsChanged();

	/**
	 * @brief Run a batch of events once.
	 *
	 * Triggers EventDispenser::processEvents() and then updates the on-screen event counter via
	 * \ref GemcGUI::updateGui "updateGui()".
	 */
	void beamOn();

	/**
	 * @brief Enable “Cycle” mode (periodic processing) and process one cycle.
	 *
	 * Starts \ref GemcGUI::gtimer "gtimer" with a 2-second interval and calls EventDispenser::processEvents().
	 * Subsequent invocations occur on each timer timeout.
	 */
	void cycleBeamOn();

	/**
	 * @brief Disable “Cycle” mode by stopping the cycle timer.
	 */
	void stopCycleBeamOn();

	/**
	 * @brief Quit the application.
	 *
	 * Requests application shutdown via \c qApp.
	 */
	void gquit();

	/**
	 * @brief Switch the active page in the right-side content area.
	 *
	 * The currently selected entry in the left button widget determines the page index applied to
	 * \ref GemcGUI::rightContent "rightContent".
	 *
	 * @param current Currently selected list item (may be null when selection changes).
	 * @param previous Previously selected list item, used as fallback when @p current is null.
	 */
	void change_page(QListWidgetItem* current, QListWidgetItem* previous);
};
