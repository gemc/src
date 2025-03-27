#ifndef GQTBUTTONSWIDGET_H
#define GQTBUTTONSWIDGET_H

#include <string>
#include <vector>
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVector>
#include <QIcon>

// Struct for storing button information
struct ButtonInfo {
	explicit ButtonInfo(const std::string &icon);

	QListWidgetItem *thisButton;
	QIcon iconForState(int state) const;

private:
	std::string buttonName;
};

class GQTButtonsWidget : public QWidget {
	Q_OBJECT

public:
	explicit GQTButtonsWidget(double h, double v,
							  const std::vector<std::string> &bicons,
							  bool vertical = true,
							  QWidget *parent = nullptr);
	~GQTButtonsWidget() override = default;

	// Exposed for external access
	QListWidget *buttonsWidget;

	// Returns the index of the button pressed
	int button_pressed() const { return buttonsWidget->currentRow(); }

	// Programmatically press a button
	void press_button(int i);

private:
	std::vector<ButtonInfo *> buttons;

private slots:
			void buttonWasPressed(QListWidgetItem *item);
};

class GQTToggleButtonWidget : public QWidget {
	Q_OBJECT

public:
	explicit GQTToggleButtonWidget(int buttonWidth, int buttonHeight, int borderRadius,
								   const std::vector<std::string> &titles,
								   bool vertical = true,
								   QWidget *parent = nullptr);

	// Returns the index of the last pressed button
	int buttonPressed() const { return buttonPressedIndex; }

	// Returns the state (checked or not) of the last pressed button
	bool lastButtonState() const;

	// Toggle a button by index
	void toggleButton(int index);

	// Returns the status (checked or not) of a button by index
	bool buttonStatus(int index) const;

	signals:
			void buttonPressedIndexChanged(int index);

private:
	int buttonPressedIndex;
	QVector<QPushButton *> buttons;

private slots:
			void setButtonPressed(int index);
};

#endif // GQTBUTTONSWIDGET_H
