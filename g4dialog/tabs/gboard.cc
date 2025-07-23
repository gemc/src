// G4Dialog
#include "gboard.h"
#include "gui_session.h"
#include "g4dialog_options.h" // Provides G4DIALOG_LOGGER constant and option definitions


// qt
#include <QTextEdit>
#include <QVBoxLayout>
#include <QString>
#include <QTextBlock>

GBoard::GBoard(GOptions* gopt, QWidget* parent)
	: QWidget(parent), log(std::make_unique<GLogger>(gopt, G4DIALOG_LOGGER, "GBoard")) {
	log->debug(CONSTRUCTOR, "GBoard");

	// --- Create top bar widgets ---
	searchLineEdit = new QLineEdit(this);
	searchLineEdit->setPlaceholderText("Filter log lines (case insensitive)...");
	searchLineEdit->setClearButtonEnabled(true); // Nice feature to easily clear search


	clearButton = new QToolButton(this);
	clearButton->setIcon(style()->standardIcon(QStyle::SP_DialogResetButton));
	clearButton->setToolTip("Clear Log");
	clearButton->setText("Clear");                                 // for some reason, the SP_TrashIcon icon is not showing, so using SP_DialogResetButton
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
	logTextEdit->setReadOnly(true);
	logTextEdit->setMinimumHeight(200);
	logTextEdit->setMinimumWidth(400);

	// Set font family that supports the symbols if needed
	//	 QFont font("Courier New", 12); // Example: Mono font often good for logs
	//	font.setFamily("Monospace");
	//	logTextEdit->setFont(font);


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

void GBoard::appendLog(const QString& htmlFragment) { // Renamed param for clarity
	if (!logTextEdit) return;


	// Ensure GUI updates happen in the GUI thread
	if (QThread::currentThread() != logTextEdit->thread()) {
		QMetaObject::invokeMethod(this, "appendLog", Qt::QueuedConnection, Q_ARG(QString, htmlFragment));
		return;
	}

	// Get document and cursor
	QTextDocument* doc    = logTextEdit->document();
	QTextCursor    cursor = logTextEdit->textCursor();

	// Determine if scrolled to bottom BEFORE inserting
	bool isAtEnd = (cursor.position() == doc->characterCount() - 1) ||
	               (logTextEdit->verticalScrollBar()->value() == logTextEdit->verticalScrollBar()->maximum());


	// Move cursor to the end always
	cursor.movePosition(QTextCursor::End);
	logTextEdit->setTextCursor(cursor);

	// Insert the HTML fragment received from GUI_Session
	// CRITICAL: Do NOT add an extra <br> here if GUI_Session is already sending line-by-line
	// Let's try inserting just the fragment first. QTextEdit might handle line breaks better itself.
	// logTextEdit->insertHtml(htmlFragment + "<br>");
	logTextEdit->insertHtml(htmlFragment); // TRY THIS FIRST

	// --- Force a paragraph break AFTER inserting (alternative to <br>) ---
	// This explicitly creates a new QTextBlock
	cursor.insertBlock(); // TRY THIS INSTEAD OF/AFTER insertHtml

	logTextEdit->setTextCursor(cursor); // Ensure cursor is at the very end

	// --- Apply Filter to the Newly Added Block (or the previous one) ---
	// Since we just inserted a block, the relevant text might be in the *previous* block
	QTextBlock targetBlock = cursor.block().previous(); // Get the block we likely just finished
	if (!targetBlock.isValid()) {                       // If it was the very first block
		targetBlock = doc->firstBlock();
	}

	if (targetBlock.isValid() && !currentFilterText.isEmpty()) {
		bool matches = targetBlock.text().indexOf(currentFilterText, 0, Qt::CaseInsensitive) >= 0;
		targetBlock.setVisible(matches);
	}
	else if (targetBlock.isValid()) {
		// Ensure visible if filter is empty
		targetBlock.setVisible(true);
	}

	// Auto-scroll to the bottom ONLY if the user was already at the bottom before insert
	if (isAtEnd) { logTextEdit->verticalScrollBar()->setValue(logTextEdit->verticalScrollBar()->maximum()); }
}

// Slot to Filter the Log View
void GBoard::filterLog(const QString& searchText) {
	if (!logTextEdit) return;

	currentFilterText       = searchText.trimmed(); // Store trimmed filter text
	QTextDocument* document = logTextEdit->document();

	// Use Qt::CaseInsensitive for case-insensitive search
	Qt::CaseSensitivity cs = Qt::CaseInsensitive;

	QTextCursor hideCursor(document); // Use a cursor for block manipulation efficiency
	hideCursor.beginEditBlock();      // Group visibility changes for performance

	for (QTextBlock block = document->begin(); block.isValid(); block = block.next()) {
		QString blockText = block.text();
		bool    contains  = (block.text().indexOf(currentFilterText, 0, cs) >= 0);
		bool    matches   = currentFilterText.isEmpty() || contains;
		block.setVisible(matches);
	}

	hideCursor.endEditBlock(); // End grouping

	// Ensure the viewport updates to reflect visibility changes
	logTextEdit->viewport()->update();

	// Optional: scroll to top after filtering might be helpful sometimes
	// logTextEdit->verticalScrollBar()->setValue(0);

	//log->debug(NORMAL, "Filter applied: '", currentFilterText.toStdString(), "'");
}

void GBoard::clearLog() {
	if (logTextEdit) {
		logTextEdit->clear();
		// Optional: Log the action itself (if desired)
		// appendLog(ansiToHtml("Log cleared by user.")); // Be careful not to cause loops
		log->info(1, "Log cleared by user."); // Log via GLogger is better
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
