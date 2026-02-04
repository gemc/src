#pragma once

// C++
#include <string>
#include <vector>

// qt
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVector>
#include <QIcon>

/**
 * \struct ButtonInfo
 * \brief Logical description of one icon-button entry used by \c GQTButtonsWidget.
 *
 * \details
 * \c ButtonInfo binds together:
 * - a base icon "name" (typically a Qt resource prefix such as \c ":/images/myButton")
 * - a \c QListWidgetItem used by the UI list view
 *
 * The helper \c iconForState() converts the base icon name into a filename using a state suffix:
 * \code
 * <base>_<state>.svg
 * \endcode
 *
 * \note
 * This struct allocates a \c QListWidgetItem in its constructor and stores it in \c thisButton.
 * Ownership and lifetime are managed by the container widget that creates these objects.
 */
struct ButtonInfo
{
	/**
	 * \brief Constructs a \c ButtonInfo for a given base icon name.
	 *
	 * \param icon Base icon name used to build the icon filenames for different states.
	 *             Typical value is a Qt resource prefix such as \c ":/images/firstButton".
	 */
	explicit ButtonInfo(const std::string& icon);

	/**
	 * \brief The UI item representing this logical button.
	 *
	 * \details
	 * The item is created in the \c ButtonInfo constructor and is later inserted into a \c QListWidget.
	 *
	 * \warning
	 * External code should treat this pointer as internal state owned by the widget; do not delete it.
	 */
	QListWidgetItem* thisButton;

	/**
	 * \brief Returns the icon for a given interaction state.
	 *
	 * \details
	 * The file name is constructed as:
	 * \code
	 * <base_icon_name>_<state>.svg
	 * \endcode
	 * If the file exists, a \c QIcon is returned. Otherwise an empty \c QIcon is returned.
	 *
	 * \param state State identifier used in the filename suffix.
	 *              Convention in this module:
	 *              - \c 1 : normal
	 *              - \c 2 : pressed
	 *
	 * \return \c QIcon for the requested state or an empty icon if the file does not exist.
	 */
	QIcon iconForState(int state) const;

private:
	std::string buttonName; ///< Base icon name used to form state-specific filenames.
};

/**
 * \class GQTButtonsWidget
 * \brief Icon-based button strip widget implemented using a \c QListWidget.
 *
 * \details
 * \c GQTButtonsWidget uses a \c QListWidget in \c QListView::IconMode as the rendering backend.
 * Each entry is a \c QListWidgetItem whose icon is updated to reflect interaction.
 *
 * Typical behavior:
 * - When a user presses an item, all items are reset to the "normal" icon (state \c 1) and the pressed
 *   item is set to the "pressed" icon (state \c 2).
 * - \c press_button() allows pressing a button programmatically.
 * - \c reset_buttons() restores all icons to the "normal" state.
 *
 * \note
 * This widget is intended for use in the Qt GUI thread.
 */
class GQTButtonsWidget : public QWidget
{
	Q_OBJECT

public:
	/**
	 * \brief Constructs a \c GQTButtonsWidget.
	 *
	 * \details
	 * The constructor:
	 * - creates one internal \c ButtonInfo per entry in \p bicons
	 * - instantiates and configures a \c QListWidget in icon mode
	 * - sets icon size based on \p h and \p v
	 * - lays out the list vertically or horizontally based on \p vertical
	 *
	 * \param h Icon width in pixels (used for \c QListWidget::setIconSize()).
	 * \param v Icon height in pixels (used for \c QListWidget::setIconSize()).
	 * \param bicons Vector of base icon names used to locate icons.
	 * \param vertical If \c true, buttons are arranged in a column; otherwise in a row.
	 * \param parent Parent widget (Qt ownership). Default is \c nullptr.
	 */
	explicit GQTButtonsWidget(double                          h, double v,
	                          const std::vector<std::string>& bicons,
	                          bool                            vertical = true,
	                          QWidget*                        parent   = nullptr);

	~GQTButtonsWidget() override = default;

	/**
	 * \brief Underlying \c QListWidget used to render the icons.
	 *
	 * \details
	 * Exposed for integration (e.g. embedding in larger GUIs, applying additional policies,
	 * connecting to more signals).
	 *
	 * \warning
	 * The widget is parented to \c this and must not be deleted externally.
	 */
	QListWidget* buttonsWidget;

	/**
	 * \brief Returns the index of the currently selected (pressed) button.
	 *
	 * \details
	 * This returns the current row in the underlying \c QListWidget. If no item is selected,
	 * Qt typically reports \c -1.
	 *
	 * \return Index of the current row in \c buttonsWidget.
	 */
	int button_pressed() const { return buttonsWidget->currentRow(); }

	/**
	 * \brief Programmatically press a button.
	 *
	 * \details
	 * This sets the current row and updates the pressed button's icon to the "pressed" state (state \c 2).
	 * It does not automatically reset other icons; if you need a clean state first, call \c reset_buttons().
	 *
	 * \param i Index of the button to press. Out-of-range values are ignored.
	 */
	void press_button(int i);

	/**
	 * \brief Reset all buttons to the "normal" icon state.
	 *
	 * \details
	 * This updates every stored \c QListWidgetItem icon to state \c 1.
	 * It does not change the current selection row.
	 */
	void reset_buttons();

private:
	/**
	 * \brief Storage for per-button metadata.
	 *
	 * \details
	 * Each entry corresponds to one icon name passed in the constructor.
	 */
	std::vector<ButtonInfo*> buttons;

private slots:
	/**
	 * \brief Handles user interaction with a list item.
	 *
	 * \details
	 * When an item is pressed:
	 * - all items are reset to the "normal" icon state (state \c 1)
	 * - the pressed item is set to the "pressed" icon state (state \c 2)
	 *
	 * \param item The \c QListWidgetItem that was pressed.
	 */
	void buttonWasPressed(QListWidgetItem* item);
};

/**
 * \class GQTToggleButtonWidget
 * \brief Group of toggleable push buttons with a tracked "last pressed" index.
 *
 * \details
 * This widget constructs one checkable \c QPushButton per title and places them into a layout selected
 * by \p vertical. When a button is clicked, the widget stores the pressed index and emits
 * \c buttonPressedIndexChanged().
 *
 * \note
 * This widget does not enforce mutual exclusivity; multiple buttons may be checked simultaneously.
 * The "pressed index" refers to the last clicked button, not the only active button.
 */
class GQTToggleButtonWidget : public QWidget
{
	Q_OBJECT

public:
	/**
	 * \brief Constructs a \c GQTToggleButtonWidget.
	 *
	 * \param buttonWidth Fixed width of each button in pixels.
	 * \param buttonHeight Fixed height of each button in pixels.
	 * \param borderRadius Border radius (in pixels) used to round button corners.
	 * \param titles Titles (labels) for the buttons.
	 * \param vertical If \c true, buttons are arranged in a column; otherwise in a row.
	 * \param parent Parent widget (Qt ownership). Default is \c nullptr.
	 */
	explicit GQTToggleButtonWidget(int                             buttonWidth, int buttonHeight, int borderRadius,
	                               const std::vector<std::string>& titles,
	                               bool                            vertical = true,
	                               QWidget*                        parent   = nullptr);

	/**
	 * \brief Returns the last pressed button index.
	 *
	 * \details
	 * If no button has been pressed yet, the value is \c -1.
	 *
	 * \return Last pressed button index.
	 */
	int buttonPressed() const { return buttonPressedIndex; }

	/**
	 * \brief Returns the checked state of the last pressed button.
	 *
	 * \details
	 * If no button has been pressed yet, or if the stored index is out of range, returns \c false.
	 *
	 * \return \c true if the last pressed button is currently checked; otherwise \c false.
	 */
	bool lastButtonState() const;

	/**
	 * \brief Toggle the checked state of a button.
	 *
	 * \param index Index of the button to toggle. Out-of-range values are ignored.
	 */
	void toggleButton(int index);

	/**
	 * \brief Returns the checked state for a specific button.
	 *
	 * \param index Index of the button to query. Out-of-range values return \c false.
	 * \return \c true if the button is checked; otherwise \c false.
	 */
	bool buttonStatus(int index) const;

	/**
	 * \brief Uncheck all buttons.
	 *
	 * \details
	 * This resets only the check state of all buttons. It does not emit \c buttonPressedIndexChanged()
	 * and it does not modify \c buttonPressedIndex.
	 */
	void reset_buttons();

signals:
	/**
	 * \brief Emitted whenever the last pressed button index changes.
	 *
	 * \param index The new pressed button index.
	 */
	void buttonPressedIndexChanged(int index);

private:
	int                   buttonPressedIndex; ///< Index of the last clicked button (\c -1 if none clicked yet).
	QVector<QPushButton*> buttons;            ///< Storage of button pointers in construction order.

private slots:
	/**
	 * \brief Internal slot used to update the last pressed index and emit the change signal.
	 *
	 * \param index Index of the button that was clicked.
	 */
	void setButtonPressed(int index);
};
