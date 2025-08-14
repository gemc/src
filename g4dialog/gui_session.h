#pragma once

#include "G4UIsession.hh"
#include "gboard.h"

// gemc
#include "gbase.h"

/**
 * @class GUI_Session
 * @brief Custom GEANT4 UI session that directs output to a Qt6 board widget.
 *
 * An instance of GUI_Session is passed to the G4UImanager so that GEANT4 output is sent
 * to the board widget rather than to the terminal or log files.
 */
class GUI_Session : public GBase<GUI_Session>, public G4UIsession {
public:
	/**
	 * @brief Constructs a new GUI_Session object.
	 * @param gopt Shared pointer to a GOptions
	 * @param board Pointer to the GBoard widget that will display the messages.
	 */
	GUI_Session(const std::shared_ptr<GOptions>& gopt, GBoard* board);


	/**
	 * @brief Receives standard output from GEANT4.
	 * @param coutString The output string.
	 * @return Always returns 0.
	 */
	G4int ReceiveG4cout(const G4String& coutString) override;

	/**
	 * @brief Receives error output from GEANT4.
	 * @param cerrString The error string.
	 * @return Always returns 0.
	 */
	G4int ReceiveG4cerr(const G4String& cerrString) override;

private:
	GBoard* board; // let parent manage GBoard
	QString          ansiToHtml(const QString& ansiText);
};
