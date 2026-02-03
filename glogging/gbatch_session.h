#pragma once

// geant4
#include "G4UIsession.hh"

// c++
#include <fstream>

/**
 * @class GBatch_Session
 * @brief Batch-mode \c G4UIsession that tees Geant4 output to files and the terminal.
 *
 * This class is designed to be passed to the Geant4 UI manager so that all \c G4cout and \c G4cerr
 * traffic is redirected through \c G4UIsession callbacks. In batch/production runs, this enables
 * persistent log capture while still keeping output visible on the console.
 *
 * \details Operational behavior
 * - On construction, the session opens two files in the current working directory:
 *   - \c gemc.log : destination for \c G4cout output
 *   - \c gemc.err : destination for \c G4cerr output
 * - When \ref GBatch_Session::ReceiveG4cout "ReceiveG4cout()" is called by Geant4:
 *   - The message is appended to \c gemc.log
 *   - The same message is forwarded to \c std::cout
 * - When \ref GBatch_Session::ReceiveG4cerr "ReceiveG4cerr()" is called by Geant4:
 *   - The message is appended to \c gemc.err
 *   - The same message is forwarded to \c std::cerr
 *
 * \note The files are held open for the lifetime of the session object. They will be closed
 * automatically when the \c std::ofstream members are destroyed.
 */
class GBatch_Session : public G4UIsession
{
public:
	/**
	 * @brief Constructs the batch session and opens the log streams.
	 *
	 * This constructor opens \c gemc.log and \c gemc.err for writing.
	 *
	 * \note The default behavior of \c std::ofstream::open for a filename is to create/truncate
	 * the file unless additional open modes are specified.
	 *
	 * \warning If the process lacks permission to create or write these files in the current
	 * working directory, the streams may fail to open. Callers can inspect stream state if
	 * they need to handle such failures explicitly.
	 */
	GBatch_Session() {
		// Open the batch log files immediately so all subsequent Geant4 output is captured.
		logFile.open("gemc.log");
		errFile.open("gemc.err");
	}

	/**
	 * @brief Receives Geant4 standard output and tees it to \c gemc.log and \c std::cout.
	 *
	 * @param coutString The message fragment provided by Geant4 for standard output.
	 * @return Always returns 0 to indicate the message was handled.
	 *
	 * \note This method flushes both the file stream and \c std::cout to keep logs consistent
	 * in long-running batch jobs.
	 */
	G4int ReceiveG4cout(const G4String& coutString) override {
		// Persist the message for post-run inspection...
		logFile << coutString << std::flush;
		// ...while also preserving real-time visibility on the terminal.
		std::cout << coutString << std::flush;
		return 0;
	}

	/**
	 * @brief Receives Geant4 standard error and tees it to \c gemc.err and \c std::cerr.
	 *
	 * @param cerrString The message fragment provided by Geant4 for standard error.
	 * @return Always returns 0 to indicate the message was handled.
	 *
	 * \note This method flushes both the file stream and \c std::cerr to reduce the chance of
	 * losing diagnostic output if the process terminates unexpectedly.
	 */
	G4int ReceiveG4cerr(const G4String& cerrString) override {
		// Persist stderr separately so errors can be grepped/triaged independently of stdout.
		errFile << cerrString << std::flush;
		std::cerr << cerrString << std::flush;
		return 0;
	}

private:
	std::ofstream logFile; ///< File stream receiving all \c G4cout output (default : \c gemc.log).
	std::ofstream errFile; ///< File stream receiving all \c G4cerr output (default : \c gemc.err).
};
