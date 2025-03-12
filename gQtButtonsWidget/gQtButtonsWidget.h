#ifndef  GQTBUTTONSWIDGET_H
#define  GQTBUTTONSWIDGET_H  1

// c++
#include <string>
#include <vector>

// qt
#include <QtWidgets>

struct ButtonInfo {
    // a button name must have 2 png files:
    // 1. "name_1.png"
    // 2. "name_2.png"
    ButtonInfo(std::string icon);

public:
    QListWidgetItem *thisButton;

    QIcon iconForState(int state);

private:
    std::string buttonName;
};


class GQTButtonsWidget : public QWidget {
    Q_OBJECT // Required for non-qt signals/slots

public:
    GQTButtonsWidget(double h, double v, std::vector <std::string> bicons, bool vertical = true, QWidget *parent = nullptr);

    // no need to delete the pointers below, done by qt parenting
    ~GQTButtonsWidget() {}

    // public so we can connect to it
    QListWidget *buttonsWidget;

    // return the button pressed
    int button_pressed() { return buttonsWidget->currentRow(); }

    void press_button(int i) {
        buttonsWidget->setCurrentRow(i);
        buttonsWidget->item(i)->setIcon(buttons[i]->iconForState(2));
    }

private:

    std::vector<ButtonInfo *> buttons;

private slots:

    void buttonWasPressed(QListWidgetItem * item);

};


class GQTToggleButtonWidget : public QWidget {
Q_OBJECT // Required for non-qt signals/slots

public:
    GQTToggleButtonWidget(int buttonWidth, int buttonHeight, int borderRadius,
                          std::vector <std::string> titles, bool vertical = true, QWidget *parent = nullptr);

    // Returns the index of the last pressed button
    int buttonPressed() const {
        return buttonPressedIndex;
    }

    // Returns the state (checked or not) of the last pressed button
    bool lastButtonState() const {
        if (buttonPressedIndex >= 0 && buttonPressedIndex < buttons.size()) {
            return buttons[buttonPressedIndex]->isChecked();
        }
        return false; // Default to false if no button has been pressed
    }

    // method to toggle a button
    void toggleButton(int index) {
        buttons[index]->toggle();
    }

    // method to return the status at index
    bool buttonStatus(int index) {
        return buttons[index]->isChecked();
    }

    signals:
        // Signal to emit the index of the button pressed
        void buttonPressedIndexChanged(int index);

private:
    int buttonPressedIndex;              // Tracks the index of the last pressed button
    QVector<QPushButton *> buttons;      // Stores all buttons for access by index
    QSignalMapper *signalMapper = new QSignalMapper(this); // Signal mapper for mapping signals

private slots:
    void setButtonPressed(int index) {
        buttonPressedIndex = index;
        emit buttonPressedIndexChanged(buttonPressedIndex);
    }

};


#endif
