#ifndef GDIALOG_BOARD_H
#define GDIALOG_BOARD_H 1

// gemc
#include "glogger.h"

// qt
#include <QtWidgets>
#include <memory>

/**
 * @class GBoard
 * @brief A Qt6 board widget used for displaying log and error output.
 *
 * The GBoard class is a QTabWidget containing two tabs, one for log messages and one for error messages.
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
        log->debug(DESTRUCTOR, "GBoard");
		delete log;
    }

    /**
     * @brief Appends a log message to the log tab.
     * @param text The log message text.
     */
    void appendLog(const QString &text);

private:
    GLogger * const log;
    QTextEdit *logTextEdit;
};

#endif
