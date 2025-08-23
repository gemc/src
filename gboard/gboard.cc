// G4Dialog
#include "gboard.h"
#include "gui_session.h"

// qt
#include <QVBoxLayout>
#include <QRegularExpression>


GBoard::GBoard(const std::shared_ptr<GOptions>& gopt, QWidget* parent)
	: QWidget(parent),
	  GBase(gopt, GBOARD_LOGGER) {
	// --- Create top bar widgets ---
	searchLineEdit = new QLineEdit(this);
	searchLineEdit->setObjectName("searchLineEdit");
	searchLineEdit->setPlaceholderText("Filter log lines (case insensitive)...");
	searchLineEdit->setClearButtonEnabled(true); // Nice feature to easily clear search

	clearButton = new QToolButton(this);
	clearButton->setObjectName("clearButton");
	clearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
	clearButton->setToolTip("Clear Log");
	clearButton->setText("Clear");
	// for some reason, the SP_TrashIcon icon is not showing, so using SP_DialogResetButton
	clearButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); // Or other style
	clearButton->setEnabled(true);

	saveButton = new QToolButton(this);
	saveButton->setIcon(style()->standardIcon(QStyle::SP_DialogSaveButton));
	saveButton->setToolTip("Save Log to File");
	saveButton->setEnabled(true);


	// Create a horizontal layout for the top bar.
	auto* topBarLayout = new QHBoxLayout;
	topBarLayout->addWidget(searchLineEdit);
	topBarLayout->addWidget(clearButton);
	topBarLayout->addWidget(saveButton);
	topBarLayout->setSpacing(5);

	// Create a QTextEdit for log messages.
	logTextEdit = new QTextEdit(this);
	logTextEdit->setAcceptRichText(true);
	logTextEdit->setReadOnly(true);
	logTextEdit->setMinimumHeight(200);
	logTextEdit->setMinimumWidth(400);

	// --- Dark theme (local to this widget) ---
	this->setStyleSheet(
	                    "QWidget { background-color: #0b0e12; color: #e6e6e6; }"
	                    "QTextEdit { background-color: #0f1115; color: #e6e6e6; }"
	                    // Override the style for the search line edit and the clear button
						"QLineEdit#searchLineEdit { background-color: #ffffff; color: #000000; }"
						"QToolButton#clearButton { background-color: #f0f0f0; color: #000000; }"	                   );

	auto* layout = new QVBoxLayout(this);
	layout->addLayout(topBarLayout);
	layout->addWidget(logTextEdit, 1); // 1: stretchable
	setLayout(layout);

	// --- Connect signals to slots ---
	connect(searchLineEdit, &QLineEdit::textChanged, this, &GBoard::filterLog);
	connect(clearButton, &QToolButton::clicked, this, &GBoard::clearLog);
	connect(saveButton, &QToolButton::clicked, this, &GBoard::saveLog);

	log->info(1, "GBoard initialized");
}

void GBoard::appendLog(const QString& htmlFragment) {
	if (htmlFragment.trimmed().isEmpty()) { return; }

	// Append to the source of truth, the full log line list
	fullLogLines.append(htmlFragment);

	// Call updateDisplay to show the new line if it matches the current filter
	updateDisplay();
}

void GBoard::updateDisplay() {
	if (!logTextEdit) return;

	// Use invokeMethod to ensure this runs on the GUI thread
	if (QThread::currentThread() != logTextEdit->thread()) {
		QMetaObject::invokeMethod(this, "updateDisplay", Qt::QueuedConnection);
		return;
	}

	logTextEdit->clear(); // Clear the QTextEdit

	const bool          filtering = !currentFilterText.isEmpty();
	Qt::CaseSensitivity cs        = Qt::CaseInsensitive;

	for (const QString& line : fullLogLines) {
		bool matches = true;

		if (filtering) { matches = (line.indexOf(currentFilterText, 0, cs) >= 0); }

		if (matches) { logTextEdit->append(line); }
	}

	// Auto-scroll to the bottom after updating the display
	logTextEdit->verticalScrollBar()->setValue(logTextEdit->verticalScrollBar()->maximum());
}

void GBoard::filterLog(const QString& searchText) {
	currentFilterText = searchText.trimmed();
	updateDisplay();
}

void GBoard::clearLog() {
	if (logTextEdit) {
		fullLogLines.clear();
		updateDisplay();
		log->info(1, "Log cleared by user.");
	}
}

// Slot to Save the Log
void GBoard::saveLog() {
	if (!logTextEdit) return;

	QString defaultFileName = "gboard_log.log"; // Suggest a default name
	QString fileName        = QFileDialog::getSaveFileName(this,
	                                                       tr("Save Log File"),
	                                                       defaultFileName, // Default file/path suggestion
	                                                       tr("Log Files (*.log);;Text Files (*.txt);;All Files (*)"));

	if (fileName.isEmpty()) {
		return; // User cancelled
	}

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr("Save Log Error"),
		                     tr("Could not open file %1 for writing:\n%2.")
		                     .arg(QDir::toNativeSeparators(fileName), file.errorString()));
		log->warning("Failed to save log to ", fileName.toStdString(), ". Error: ", file.errorString().toStdString());
		return;
	}

	QTextStream out(&file);
	// Save the plain text content (most common for logs)
	// This automatically includes *all* lines, even hidden ones
	out << logTextEdit->toPlainText();
	file.close(); // Stream destructor closes it, but explicit is fine

	log->info("Log saved successfully to ", fileName.toStdString());
	// Optional: Show a status bar message or brief confirmation dialog
}
