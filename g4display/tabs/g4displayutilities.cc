// g4display
#include "g4displayutilities.h"

// c++
#include <string>
using namespace std;


G4DisplayUtilities::G4DisplayUtilities([[maybe_unused]] const std::shared_ptr<GOptions>& gopt,  	std::shared_ptr<GLogger> logger, QWidget* parent) : QWidget(parent), log(logger) {
	log->debug(CONSTRUCTOR, "G4DisplayUtilities");

	// Create a QVBoxLayout for the main layout
	// Create a QTextEdit for log messages.
	auto logTextEdit = new QTextEdit(this);
	logTextEdit->setReadOnly(true);
	logTextEdit->setText("Log Board Initialized");
	logTextEdit->setStyleSheet("background-color: lightblue;");
	logTextEdit->setMinimumHeight(200);
	logTextEdit->setMinimumWidth(400);

}
