#ifndef GQTBUTTONSWIDGET_H
#define GQTBUTTONSWIDGET_H

#include <string>
#include <vector>
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVector>
#include <QIcon>

/**
 * \struct ButtonInfo
 * \brief Holds information for a button including its icon.
 *
 * This structure stores a pointer to a QListWidgetItem and a base icon name.
 * The method \c iconForState() returns a QIcon corresponding to a given state.
 */
struct ButtonInfo {
	/**
	 * \brief Constructs a ButtonInfo with the given base icon name.
	 * \param icon Base icon name (used to build filenames for different states).
	 */
	explicit ButtonInfo(const std::string &icon);

	QListWidgetItem *thisButton; ///< Pointer to the QListWidgetItem representing the button.

	/**
	 * \brief Returns the icon for a given state.
	 *
	 * Constructs a filename by appending an underscore and the state number (e.g. "_1" for normal)
	 * to the base icon name. If the file exists, returns a QIcon; otherwise, returns an empty QIcon.
	 *
	 * \param state State identifier (1 for normal, 2 for pressed, etc.).
	 * \return QIcon corresponding to the state.
	 */
	QIcon iconForState(int state) const;

private:
	std::string buttonName; ///< Base icon name.
};

/**
 * \class GQTButtonsWidget
 * \brief A widget displaying a list of icon-based buttons.
 *
 * GQTButtonsWidget uses a QListWidget to display buttons in icon mode.
 * It supports vertical or horizontal layout and allows external access to the
 * underlying QListWidget.
 */
class GQTButtonsWidget : public QWidget {
	Q_OBJECT

public:
	/**
	 * \brief Constructs a GQTButtonsWidget.
	 *
	 * \param h The width (icon width) for each button.
	 * \param v The height (icon height) for each button.
	 * \param bicons A vector of base icon names for the buttons.
	 * \param vertical If true, lays out buttons vertically; if false, horizontally.
	 * \param parent Pointer to the parent widget (default is nullptr).
	 */
	explicit GQTButtonsWidget(double h, double v,
							  const std::vector<std::string> &bicons,
							  bool vertical = true,
							  QWidget *parent = nullptr);
	~GQTButtonsWidget() override = default;

	QListWidget *buttonsWidget; ///< Exposed QListWidget for external access.

	/**
	 * \brief Returns the index of the currently pressed button.
	 * \return The index of the pressed button.
	 */
	int button_pressed() const { return buttonsWidget->currentRow(); }

	/**
	 * \brief Programmatically press a button.
	 *
	 * Sets the current row and updates the button's icon to the pressed state.
	 *
	 * \param i Index of the button to press.
	 */
	void press_button(int i);

private:
	std::vector<ButtonInfo *> buttons; ///< Vector holding pointers to ButtonInfo objects.

private slots:
			/**
			 * \brief Slot triggered when a button is pressed.
			 *
			 * Resets all buttons to their normal state and sets the pressed button's icon to the pressed state.
			 *
			 * \param item Pointer to the QListWidgetItem that was pressed.
			 */
			void buttonWasPressed(QListWidgetItem *item);
};

/**
 * \class GQTToggleButtonWidget
 * \brief A widget providing a collection of toggleable buttons.
 *
 * GQTToggleButtonWidget creates a set of checkable QPushButtons arranged either vertically or horizontally.
 * It tracks the last pressed button, provides methods to toggle buttons, and emits a signal when the pressed
 * button index changes.
 */
class GQTToggleButtonWidget : public QWidget {
	Q_OBJECT

public:
	/**
	 * \brief Constructs a GQTToggleButtonWidget.
	 *
	 * \param buttonWidth The fixed width for each button.
	 * \param buttonHeight The fixed height for each button.
	 * \param borderRadius The border radius for rounded button corners.
	 * \param titles A vector of titles for the buttons.
	 * \param vertical If true, arranges buttons vertically; if false, horizontally.
	 * \param parent Pointer to the parent widget (default is nullptr).
	 */
	explicit GQTToggleButtonWidget(int buttonWidth, int buttonHeight, int borderRadius,
								   const std::vector<std::string> &titles,
								   bool vertical = true,
								   QWidget *parent = nullptr);

	/**
	 * \brief Returns the index of the last pressed button.
	 * \return The index of the last pressed button.
	 */
	int buttonPressed() const { return buttonPressedIndex; }

	/**
	 * \brief Returns the state (checked or not) of the last pressed button.
	 * \return True if the last pressed button is checked; false otherwise.
	 */
	bool lastButtonState() const;

	/**
	 * \brief Toggles the button at the specified index.
	 * \param index The index of the button to toggle.
	 */
	void toggleButton(int index);

	/**
	 * \brief Returns the status (checked or not) of a button.
	 * \param index The index of the button.
	 * \return True if the button is checked; false otherwise.
	 */
	bool buttonStatus(int index) const;

	signals:
			/**
			 * \brief Signal emitted when the pressed button index changes.
			 * \param index The new pressed button index.
			 */
			void buttonPressedIndexChanged(int index);

private:
	int buttonPressedIndex;         ///< Index of the last pressed button.
	QVector<QPushButton *> buttons; ///< Container for toggleable QPushButtons.

private slots:
			/**
			 * \brief Slot to set the currently pressed button.
			 * \param index The index of the pressed button.
			 */
			void setButtonPressed(int index);
};

#endif // GQTBUTTONSWIDGET_H
