#pragma once

// gemc
#include "gbase.h"

// qt
#include <QtWidgets>
#include <memory>

constexpr const char* GBOARD_LOGGER = "gboard";

/**
 * @namespace gboard
 * @brief Helper namespace for the gboard module.
 *
 * This namespace currently hosts small configuration helpers (such as the module option definition).
 * Keeping these in a namespace avoids polluting the global symbol space.
 */
namespace gboard {
/**
 * @brief Defines the module options used by the gboard components.
 *
 * The returned options are rooted at the module logger name (\c GBOARD_LOGGER).
 * The concrete set of switches/scalars is populated by the underlying GOptions infrastructure.
 *
 * @return A fully constructed GOptions instance for this module.
 */
inline GOptions defineOptions() {
	GOptions goptions(GBOARD_LOGGER);
	return goptions;
}
}


/**
 * @class GBoard
 * @brief A Qt widget that displays read-only log text with a compact "top bar" UI.
 *
 * GBoard is intended to be used as a log sink and viewer:
 * - **Append-only ingestion**: new log lines are added through \ref GBoard::appendLog "appendLog()".
 * - **Filtering**: a search field filters the displayed content without losing the underlying log history.
 * - **User actions**: clear (remove all stored lines) and save (export to a file).
 *
 * ### Data model and display strategy
 * - \c fullLogLines is the **source of truth** for all received log lines (stored as HTML fragments).
 * - The visible \c QTextEdit is rebuilt from \c fullLogLines whenever the filter changes or a new line arrives.
 *
 * ### Threading notes
 * In typical usage, log lines may arrive from non-GUI threads. The implementation ensures the
 * actual QTextEdit update is executed on the GUI thread.
 *
 * ### Ownership and lifetime
 * This widget owns its Qt child widgets (created with \c this as parent).
 */
class GBoard : public QWidget, public GBase<GBoard>
{
	Q_OBJECT

public:
	/**
	 * @brief Constructs a new GBoard widget.
	 *
	 * The widget instantiates its internal UI:
	 * - search line edit (filter)
	 * - clear button
	 * - save button
	 * - log display area (\c QTextEdit, read-only, rich-text enabled)
	 *
	 * @param gopt Shared pointer to the module options/logging configuration.
	 * @param parent Parent widget (default is nullptr).
	 */
	explicit GBoard(const std::shared_ptr<GOptions>& gopt, QWidget* parent = nullptr);

	GBoard(const GBoard&)            = delete;
	GBoard& operator=(const GBoard&) = delete;
	GBoard(GBoard&&)                 = delete;
	GBoard& operator=(GBoard&&)      = delete;

	/**
	 * @brief Appends a log line to the internal history and updates the display.
	 *
	 * The input is treated as an **HTML fragment** (not a full HTML document):
	 * - It is stored verbatim into \c fullLogLines.
	 * - The display is refreshed so the new line becomes visible immediately if it matches the current filter.
	 *
	 * Empty or whitespace-only fragments are ignored.
	 *
	 * @param text HTML fragment representing one log line.
	 */
	void appendLog(const QString& text);

private
slots :
	/**
	 * @brief Updates the current filter and refreshes the visible log view.
	 *
	 * Filtering is case-insensitive and matches by substring search in the stored HTML fragment.
	 * The filter is applied against \c fullLogLines; no data is discarded by filtering.
	 *
	 * @param filterText User-entered text used to filter log lines.
	 */
	void filterLog(const QString& filterText);

	/**
	 * @brief Clears the entire log history and refreshes the display.
	 *
	 * This removes all stored lines from \c fullLogLines and empties the visible \c QTextEdit.
	 */
	void clearLog();

	/**
	 * @brief Saves the currently displayed log to disk via a file dialog.
	 *
	 * The export is written as **plain text** using \c QTextEdit::toPlainText().
	 * This ensures the saved file is a conventional log format even when the display uses rich text.
	 */
	void saveLog();

	/**
	 * @brief Rebuilds the visible log view from the stored history.
	 *
	 * This method:
	 * - Ensures execution on the GUI thread.
	 * - Clears the \c QTextEdit.
	 * - Re-appends all stored lines that match the current filter.
	 * - Scrolls to the bottom after update.
	 *
	 * This is an internal method; callers should typically use higher-level operations such as
	 * \ref GBoard::appendLog "appendLog()" or \ref GBoard::filterLog "filterLog()".
	 */
	void updateDisplay();

private:
	/**
	 * @brief Search field used to capture the active filter string.
	 *
	 * Owned by this widget (Qt parent is \c this).
	 */
	QLineEdit* searchLineEdit{}; ///< Input field for search/filter text

	/**
	 * @brief Clears stored log lines and refreshes the UI.
	 *
	 * Owned by this widget (Qt parent is \c this).
	 */
	QToolButton* clearButton{}; ///< Button to clear the log

	/**
	 * @brief Opens the save dialog and writes the current log view to disk.
	 *
	 * Owned by this widget (Qt parent is \c this).
	 */
	QToolButton* saveButton{}; ///< Button to save the log

	/**
	 * @brief The main read-only log view.
	 *
	 * Rich text is enabled to support colored/formatted output (for example from ANSI conversion).
	 * Owned by this widget (Qt parent is \c this).
	 */
	QTextEdit* logTextEdit{}; ///< The main text area for displaying logs

	/**
	 * @brief Stored full history of received log lines (HTML fragments).
	 *
	 * This is the authoritative storage used for filtering and rebuilding the display.
	 */
	QStringList fullLogLines;

	/**
	 * @brief Active filter string (trimmed).
	 *
	 * When empty, all log lines are displayed. When non-empty, only matching lines are shown.
	 */
	QString currentFilterText{};
};
