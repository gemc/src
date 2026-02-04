#pragma once

#include "G4UIsession.hh"
#include "gboard.h"

// gemc
#include "gbase.h"

/**
 * @class GUI_Session
 * @brief A custom Geant4 UI session that forwards Geant4 text output to a GBoard widget.
 *
 * This class implements the Geant4 UI session interface (\c G4UIsession) so that:
 * - Standard output produced via Geant4 UI mechanisms is routed into a GUI log board.
 * - Error output is routed similarly.
 *
 * Typical usage:
 * - Construct a GUI_Session with a valid GBoard pointer.
 * - Register it with \c G4UImanager as the cout destination.
 * - As Geant4 emits output, this session converts it into rich text and appends it to the board.
 *
 * ### Ownership
 * GUI_Session does **not** own the GBoard; the caller controls the board lifetime.
 *
 * ### Formatting
 * The implementation converts common ANSI formatting sequences into HTML suitable for \c QTextEdit rich text:
 * - foreground colors 30–37
 * - bold
 * - underline
 * - reset
 */
class GUI_Session : public GBase<GUI_Session>, public G4UIsession
{
public:
	/**
	 * @brief Constructs a GUI_Session.
	 *
	 * The constructor registers this instance as the Geant4 cout destination via \c G4UImanager.
	 *
	 * @param gopt Shared pointer to the module options/logging configuration.
	 * @param board Pointer to the GBoard widget that will display forwarded messages (not owned).
	 */
	GUI_Session(const std::shared_ptr<GOptions>& gopt, GBoard* board);

	/**
	 * @brief Receives standard output from Geant4 and forwards it to the board.
	 *
	 * The incoming string may contain multiple lines. The implementation splits the text into lines,
	 * converts each line into rich text, and appends it to the board.
	 *
	 * @param coutString The standard output string emitted by Geant4.
	 * @return Always returns 0 (as required by \c G4UIsession interface).
	 */
	G4int ReceiveG4cout(const G4String& coutString) override;

	/**
	 * @brief Receives error output from Geant4 and forwards it to the board.
	 *
	 * The incoming string may contain multiple lines. The implementation splits the text into lines,
	 * converts each line into rich text, and appends it to the board.
	 *
	 * @param cerrString The error output string emitted by Geant4.
	 * @return Always returns 0 (as required by \c G4UIsession interface).
	 */
	G4int ReceiveG4cerr(const G4String& cerrString) override;

	/**
	 * @brief Destructor.
	 *
	 * Detaches this session from \c G4UImanager to avoid callbacks into a destroyed object.
	 */
	~GUI_Session() override;

private:
	/**
	 * @brief Target board used to display messages (not owned).
	 *
	 * The widget is expected to outlive this session instance. If the pointer is null,
	 * incoming output is ignored.
	 */
	GBoard* board; // let parent manage GBoard, do not kill

	/**
	 * @brief Converts ANSI escape sequences to HTML for rich text display.
	 *
	 * This helper:
	 * - Escapes regular text safely for HTML insertion.
	 * - Recognizes common ANSI SGR codes (colors, bold, underline, reset).
	 * - Produces a minimal HTML fragment suitable for appending into \c QTextEdit.
	 *
	 * This is a private helper; callers should rely on the public ReceiveG4cout/ReceiveG4cerr API.
	 *
	 * @param ansiText Text that may include ANSI escape sequences.
	 * @return An HTML fragment representing the styled text.
	 */
	QString ansiToHtml(const QString& ansiText);
};
