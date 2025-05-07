#pragma once

// gemc
#include "glogger.h"

// qt
#include <QtWidgets>
#include <memory>

/**
 * @class GBoard
 * @brief A widget that displays read-only log text along with a top bar for search, clear, and save actions.
 *
 * GBoard contains a QLineEdit for searching, a clear (trashcan) button to remove all log entries, and a save button
 * to write the log to disk. The log messages are maintained in a QStringList to support filtering.
 */
class GBoard : public QWidget {
Q_OBJECT

public:
	/**
	 * @brief Constructs a new GBoard widget.
	 * @param log A unique pointer to a GLogger for debug messages.
	 * @param parent The parent widget (default is nullptr).
	 */
	explicit GBoard(GOptions *gopt, QWidget *parent = nullptr);

	/**
	 * @brief Destructor.
	 */
	~GBoard() override {
		if (log) { // Check if log is not null before using
			log->debug(DESTRUCTOR, "GBoard");
			delete log;
		}
	}

	/**
	 * @brief Appends a log message to the log tab.
	 * @param text The log message text.
	 */
	void appendLog(const QString &text);

private slots:

	/**
	 * @brief Filters the displayed log lines based on the search text.
	 * @param filterText The text used to filter log lines.
	 */
	void filterLog(const QString &filterText);

	/**
	 * @brief Clears all log messages.
	 */
	void clearLog();

	/**
	 * @brief Opens a file dialog and saves the log to disk.
	 */
	void saveLog();

private:
	GLogger *const log;
	QLineEdit *searchLineEdit;  ///< Input field for search/filter text
	QToolButton *clearButton;   ///< Button to clear the log
	QToolButton *saveButton;    ///< Button to save the log
	QTextEdit *logTextEdit;     ///< The main text area for displaying logs

	// Maintains all log lines for filtering purposes.
	QString currentFilterText;

};

