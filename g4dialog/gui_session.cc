#include "gui_session.h"
#include "g4dialog_options.h" // Provides G4DIALOG_LOGGER constant and option definitions
#include <QRegularExpression>

GUI_Session::GUI_Session(const std::shared_ptr<GOptions>& gopt, std::shared_ptr<GBoard> b) : board(b), log(std::make_unique<GLogger>(gopt, G4DIALOG_LOGGER, "GBoard")) {
	log->debug(CONSTRUCTOR, "GUI_Session");
	log->info(0, " g4 dialog : GUI_Session created");
}

G4int GUI_Session::ReceiveG4cout(const G4String& coutString) {
	if (board) {
		QString fullQString = QString::fromStdString(coutString);

		// --- Define and Validate Regex ---
		// Split the QString into lines based on newline characters
		// Use Qt::KeepEmptyParts if you want empty lines preserved,
		// Use Qt::SkipEmptyParts to ignore lines that are just whitespace after splitting.
		// Using KeepEmptyParts is generally safer for logs.
		// Note: '\n' might need to be adjusted if G4 uses different line endings (\r\n)
		// but '\n' usually works on most platforms. QRegularExpression can handle \r?\n
		QRegularExpression lineBreakRegex("\r?\n|\u2028");
		QStringList        lines = fullQString.split(lineBreakRegex, Qt::KeepEmptyParts);

		for (const QString& line : lines) {
			QString htmlLine = ansiToHtml(line);
			board->appendLog(htmlLine);
		}
	}
	return 0;
}


G4int GUI_Session::ReceiveG4cerr(const G4String& cerrString) {
	if (board) {
		QString fullQString = QString::fromStdString(cerrString);

		// Use the same robust regex for splitting error output
		QRegularExpression lineBreakRegex("\\r?\\n|\\u2028");
		QStringList        lines = fullQString.split(lineBreakRegex, Qt::KeepEmptyParts);

		for (const QString& line : lines) {
			QString htmlLine = ansiToHtml(line);
			board->appendLog(htmlLine);
		}
	}
	return 0;
}


// Helper function to convert ANSI escape codes to HTML
// This version handles colors (30-37), bold (1), underline (4), and reset (0).
// It also processes combined codes like [1;31m.
QString GUI_Session::ansiToHtml(const QString& ansiText) {
	QString htmlText;
	htmlText.reserve(ansiText.length() * 1.2); // Pre-allocate buffer slightly larger

	// State variables
	bool    isBold       = false;
	bool    isUnderlined = false;
	QString currentColor = ""; // Store the HTML color name/code

	// ANSI color code to HTML color map
	QMap<int, QString> colorMap;
	colorMap[30] = "black";
	colorMap[31] = "red";
	colorMap[32] = "green";
	colorMap[33] = "darkorange"; // Or "yellow" - adjust as needed for visibility
	colorMap[34] = "blue";
	colorMap[35] = "magenta";
	colorMap[36] = "cyan";
	colorMap[37] = "grey"; // Or "white" - adjust as needed

	// Regex to find ANSI escape sequences: \x1B[ followed by numbers/semicolons, ending in m
	QRegularExpression ansiRegex("\x1B\\[([0-9;]*)m");

	int                             lastPos = 0;
	QRegularExpressionMatchIterator i       = ansiRegex.globalMatch(ansiText);

	while (i.hasNext()) {
		QRegularExpressionMatch match      = i.next();
		int                     currentPos = match.capturedStart();

		// 1. Append text segment before the matched ANSI code, escaping it
		if (currentPos > lastPos) {
			QString textSegment = ansiText.mid(lastPos, currentPos - lastPos);
			// Use toHtmlEscaped for robust escaping of <, >, &
			htmlText += textSegment.toHtmlEscaped();
		}

		// 2. Process the ANSI code(s)
		QString     codesStr = match.captured(1); // The part between [ and m
		QStringList codes    = codesStr.split(';', Qt::SkipEmptyParts);

		if (codes.isEmpty()) {
			// Handle CSI m (equivalent to CSI 0 m - reset)
			codes.append("0");
		}

		// Temporary state for processing multiple codes in one sequence
		bool    nextBold       = isBold;
		bool    nextUnderlined = isUnderlined;
		QString nextColor      = currentColor;
		bool    resetDetected  = false;

		for (const QString& codeStr : codes) {
			bool ok;
			int  code = codeStr.toInt(&ok);
			if (!ok) continue; // Skip invalid codes

			if (code == 0) { // Reset all attributes
				resetDetected  = true;
				nextBold       = false;
				nextUnderlined = false;
				nextColor      = "";
				// Important: Reset applies to *all* subsequent codes in the sequence too,
				// so we break the inner loop after applying the effects of the reset below.
				break;
			}
			else if (code == 1) { // Bold
				nextBold = true;
			}
			else if (code == 4) { // Underline
				nextUnderlined = true;
			}
			else if (code == 22) { // Normal intensity (neither bold nor faint)
				nextBold = false;  // Turn off bold
			}
			else if (code == 24) {      // Not underlined
				nextUnderlined = false; // Turn off underline
			}
			else if (code >= 30 && code <= 37) {      // Foreground color
				nextColor = colorMap.value(code, ""); // Get HTML color, empty if unknown
			}
			else if (code == 39) { // Default foreground color
				nextColor = "";
			}
			// Ignore other codes (background colors 40-47, faint 2, italic 3, etc.)
		}

		// 3. Apply state changes by closing/opening tags *only* if state changed

		// Close tags in reverse order if they are being turned off or changed
		if (isUnderlined && !nextUnderlined) { htmlText += "</u>"; }
		if (isBold && !nextBold) { htmlText += "</b>"; }
		if (!currentColor.isEmpty() && currentColor != nextColor) { htmlText += "</font>"; }
		// If reset was detected, ensure all current tags are closed
		if (resetDetected) {
			if (isUnderlined) htmlText += "</u>";
			if (isBold) htmlText += "</b>";
			if (!currentColor.isEmpty()) htmlText += "</font>";
		}


		// Open tags in normal order if they are being turned on or changed
		if (!currentColor.isEmpty() && currentColor != nextColor) {
			// Already closed above, now open the new one if needed
			if (!nextColor.isEmpty()) { htmlText += QString("<font color=\"%1\">").arg(nextColor); }
		}
		else if (currentColor.isEmpty() && !nextColor.isEmpty()) {
			// Was default, now has color
			htmlText += QString("<font color=\"%1\">").arg(nextColor);
		}

		if (!isBold && nextBold) { htmlText += "<b>"; }
		if (!isUnderlined && nextUnderlined) { htmlText += "<u>"; }

		// Update current state
		isBold       = nextBold;
		isUnderlined = nextUnderlined;
		currentColor = nextColor;

		// Update position for next segment
		lastPos = match.capturedEnd();
	}

	// Append any remaining text after the last ANSI code
	if (lastPos < ansiText.length()) {
		QString textSegment = ansiText.mid(lastPos);
		htmlText += textSegment.toHtmlEscaped();
	}

	// Close any remaining open tags at the very end
	if (isUnderlined) { htmlText += "</u>"; }
	if (isBold) { htmlText += "</b>"; }
	if (!currentColor.isEmpty()) { htmlText += "</font>"; }

	return htmlText;
}
