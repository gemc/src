// G4Dialog
#include "gboard.h"
#include "../gui_session.h"
#include "../g4dialog_options.h" // Provides G4DIALOG_LOGGER constant and option definitions



// qt
#include <QTextEdit>
#include <QVBoxLayout>

GBoard::GBoard(GOptions *gopt, QWidget *parent)
		: QWidget(parent), log(new GLogger(gopt, G4DIALOG_LOGGER)) {

	log->debug(CONSTRUCTOR, "GBoard");


	// Create a QTextEdit for log messages.
	logTextEdit = new QTextEdit(this);
	logTextEdit->setReadOnly(true);
	logTextEdit->setText("Log Board Initialized 1");
	logTextEdit->setMinimumHeight(200);
	logTextEdit->setMinimumWidth(400);

}

void GBoard::appendLog(const QString &text) {
	if (logTextEdit) {
		logTextEdit->append(text);
	}
}
