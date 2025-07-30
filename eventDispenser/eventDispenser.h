#pragma once

#include "goptions.h"
#include "gdynamicdigitization.h"

#include <map>
#include <vector>
#include <string>

/**
 * \class EventDispenser
 * \brief Distributes events among runs and manages dynamic digitization plugins.
 *
 * This class reads run configurations and distributes events according to weighted
 * runs. It also initializes and calls GDynamicDigitization plugins for each run, logs
 * useful information, and provides an API to query run event data.
 */
class EventDispenser {
public:
	/**
	 * \brief Constructs an EventDispenser.
	 *
	 * \param gopt Pointer to a GOptions object containing configuration parameters.
	 * \param gdynamicDigitizationMap unordered_map<std::string, shared_ptr<GDynamicDigitization>>
	 */
	EventDispenser(const std::shared_ptr<GOptions>& gopt, std::shared_ptr<const gdynamicdigitization::dRoutinesMap> gdynamicDigitizationMap);

	/**
	 * \brief Destructor for EventDispenser.
	 *
	 * Logs a debug message upon destruction.
	 */
	~EventDispenser() {
		log->debug(DESTRUCTOR, "EventDispenser");
	}

private:
	std::shared_ptr<GLogger> log;        ///< Logger instance for logging messages.

	// Parameters extracted from GOptions:
	int neventsToProcess;  ///< Total number of events to process.
	int userRunno;         ///< User-defined run number.
	int nEventBuffer;      ///< Number of events stored in the buffer.
	int currentRunno = -1; ///< Current run number being processed.
	// int elog{};              ///< Logging level for events.
	std::string variation; ///< Variation string for configuration.

	// Internal data structures:
	std::map<int, double> runWeights;  ///< Map of run numbers with associated weights.
	std::map<int, int> runEvents;        ///< Map of number of events per run (derived from weights).
	std::vector<int> listOfRuns;         ///< List of run numbers to process.

	/**
	 * \brief Fills the runEvents map based on the number of events to process.
	 *
	 * \param nevents_to_process Total number of events that need to be processed.
	 */
	void distributeEvents(int nevents_to_process);

	int currentRunIndex{}; ///< Index for the current run in listOfRuns.

	 std::shared_ptr<const gdynamicdigitization::dRoutinesMap> gDigitizationMap; ///< map of shared pointers to GDynamicDigitization plugins.

	/**
	 * \brief Increments the current run index.
	 */
	void setNextRun() { currentRunIndex++; }

public:
	/**
	 * \brief Returns the map of run events.
	 *
	 * \return A map where keys are run numbers and values are the number of events for each run.
	 */
	std::map<int, int> getRunEvents() { return runEvents; }

	/**
	 * \brief Returns the current run number.
	 *
	 * \return The current run number from the list of runs.
	 */
	[[nodiscard]] int getCurrentRun() const { return listOfRuns[currentRunIndex]; }

	/**
	 * \brief Computes and returns the total number of events across all runs.
	 *
	 * \return The sum of events across all runs.
	 */
	int getTotalNumberOfEvents() const;

	/**
	 * \brief Processes events by initializing dynamic digitization plugins and running simulations.
	 *
	 * This method initiates all available GDynamicDigitization plugins for each run,
	 * performs the simulation (e.g., beamOn) for each run, and logs detailed information
	 * if verbosity is enabled.
	 *
	 * \return An integer status code indicating the result of event processing.
	 */
	int processEvents();

	/**
	 * \brief Sets the total number of events to process.
	 *
	 * This method is public because it can be set by the GUI
	 * \param nevts The total number of events.
	 */
	void setNumberOfEvents(int nevts);
};

