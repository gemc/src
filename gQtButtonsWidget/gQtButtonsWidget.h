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

    QIcon buttonForState(int state);

private:
    std::string buttonName;
};


class GQTButtonsWidget : public QWidget {
    Q_OBJECT

public:
    GQTButtonsWidget(double h, double v, std::vector <std::string> bicons, bool vertical = true, QWidget *parent = Q_NULLPTR);

    // no need to delete the pointers below, done by qt parenting
    ~GQTButtonsWidget() {}

    // public so we can connect to it
    QListWidget *buttonsWidget;

    // return the button pressed
    int button_pressed() { return buttonsWidget->currentRow(); }

    void press_button(int i) {
        buttonsWidget->setCurrentRow(i);
        buttonsWidget->item(i)->setIcon(buttons[i]->buttonForState(2));
    }

private:

    bool is_vertical;
    std::vector<ButtonInfo *> buttons;

private slots:

    void buttonWasPressed(QListWidgetItem * item);

};

#endif
