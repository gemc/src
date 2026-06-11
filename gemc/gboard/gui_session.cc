#include "gui_session.h"

// geant4
#include "G4UImanager.hh"
#include "G4coutDestination.hh"

GUI_Session::GUI_Session(const std::shared_ptr<GOptions>& gopt, GBoard* b) :
	GBase(gopt, GBOARD_LOGGER),
	board(b) {
	// Route Geant4 UI output to this session instance so we can forward it to the GUI board.
	// SetCoutDestination updates the per-thread stream buffer but does NOT update
	// masterG4coutDestination, which G4UIQt set to itself in its constructor.
	// Worker threads use G4MasterForwardcoutDestination -> masterG4coutDestination, so
	// we must update it here to prevent G4UIQt::ReceiveG4cerr from being called on a
	// background thread (which would trigger an NSAlert from a non-main thread on macOS).
	G4UImanager::GetUIpointer()->SetCoutDestination(this);
	G4coutDestination::masterG4coutDestination = this;

	log->info(1, SFUNCTION_NAME, " g4 dialog : GUI_Session created");
}

G4int GUI_Session::ReceiveG4cout(const G4String& coutString) {
	// See header for API docs.
	if (!board) return 0;

	QString fullQString = QString::fromStdString(coutString);
	// Split into lines so that the board gets "log-like" incremental entries.
	fullQString.replace("\r\n", "\n");
	fullQString.replace('\r', '\n');
	fullQString.replace(QChar(0x2028), '\n');
	QStringList lines = fullQString.split('\n', Qt::KeepEmptyParts);

	// Convert ANSI to HTML on the calling thread (no Qt object access needed).
	QStringList htmlLines;
	htmlLines.reserve(lines.size());
	for (const QString& line : lines) { htmlLines << ansiToHtml(line); }

	// Post widget update to the main thread. Qt::AutoConnection calls directly
	// when already on the main thread, and queues the call otherwise, preventing
	// NSWindow/NSAlert operations from a Geant4 worker thread on macOS.
	auto* b = board;
	QMetaObject::invokeMethod(b, [b, htmlLines]() {
		for (const QString& htmlLine : htmlLines) { b->appendLog(htmlLine); }
	});
	return 0;
}


G4int GUI_Session::ReceiveG4cerr(const G4String& cerrString) {
	// See header for API docs.
	if (!board) return 0;

	QString fullQString = QString::fromStdString(cerrString);
	fullQString.replace("\r\n", "\n");
	fullQString.replace('\r', '\n');
	fullQString.replace(QChar(0x2028), '\n');
	QStringList lines = fullQString.split('\n', Qt::KeepEmptyParts);

	QStringList htmlLines;
	htmlLines.reserve(lines.size());
	for (const QString& line : lines) { htmlLines << ansiToHtml(line); }

	auto* b = board;
	QMetaObject::invokeMethod(b, [b, htmlLines]() {
		for (const QString& htmlLine : htmlLines) { b->appendLog(htmlLine); }
	});
	return 0;
}


// Helper function to convert ANSI escape codes to HTML.
// This version handles colors (30-37), bold (1), underline (4), and reset (0).
// It also processes combined codes like [1;31m].
//
// Design notes:
// - Input text is HTML-escaped to ensure it is safe to insert into rich-text widgets.
// - Formatting state is tracked and translated into minimal tag open/close operations.
QString GUI_Session::ansiToHtml(const QString& ansiText) {
	QString htmlText;
	htmlText.reserve(ansiText.length() * 1.2); // Pre-allocate buffer slightly larger

	// State variables
	bool    isBold       = false;
	bool    isUnderlined = false;
	QString currentColor = ""; // Store the HTML color name/code

	// ANSI color code to the HTML color map
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

		// 1. Append the text segment before the matched ANSI code, escaping it.
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

			if (code == 0) {
				// Reset all attributes
				resetDetected  = true;
				nextBold       = false;
				nextUnderlined = false;
				nextColor      = "";

				// Reset applies to all further codes in the same sequence, so stop here.
				break;
			}
			else if (code == 1) {
				// Bold
				nextBold = true;
			}
			else if (code == 4) {
				// Underline
				nextUnderlined = true;
			}
			else if (code == 22) {
				// Normal intensity (neither bold nor faint)
				nextBold = false; // Turn off bold
			}
			else if (code == 24) {
				// Not underlined
				nextUnderlined = false; // Turn off underline
			}
			else if (code >= 30 && code <= 37) {
				// Foreground color
				nextColor = colorMap.value(code, ""); // Get HTML color, empty if unknown
			}
			else if (code == 39) {
				// Default foreground color
				nextColor = "";
			}
			// Ignore other codes (background colors 40-47, faint 2, italic 3, etc.)
		}

		// 3. Apply state changes by closing/opening tags only if the state changed.

		// Close tags in reverse order if they are being turned off or changed
		if (isUnderlined && !nextUnderlined) { htmlText += "</u>"; }
		if (isBold && !nextBold) { htmlText += "</b>"; }
		if (!currentColor.isEmpty() && currentColor != nextColor) { htmlText += "</font>"; }

		// If reset was detected, ensure all current tags are closed.
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

		// Update position for the next segment
		lastPos = match.capturedEnd();
	}

	// Append any remaining text after the last ANSI code.
	if (lastPos < ansiText.length()) {
		QString textSegment = ansiText.mid(lastPos);
		htmlText            += textSegment.toHtmlEscaped();
	}

	// Close any remaining open tags at the very end.
	if (isUnderlined) { htmlText += "</u>"; }
	if (isBold) { htmlText += "</b>"; }
	if (!currentColor.isEmpty()) { htmlText += "</font>"; }

	return htmlText;
}


GUI_Session::~GUI_Session() {
	// See header for API docs.
	// Detach Geant4 cout/cerr from our GUI session (avoid dangling callback).
	if (auto* UIM = G4UImanager::GetUIpointer()) {
		UIM->SetCoutDestination(nullptr);
	}
	// Also clear masterG4coutDestination, which we claimed in the constructor.
	// Worker threads read this pointer at call time, so nulling it here prevents
	// any in-flight forwarding from reaching the already-destroyed session.
	if (G4coutDestination::masterG4coutDestination == this) {
		G4coutDestination::masterG4coutDestination = nullptr;
	}
}
