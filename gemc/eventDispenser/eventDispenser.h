#pragma once

#include <gemc/gbase/gbase.h>
#include <gemc/ganalysis/gAnalysisAccumulator.h>
#include <gemc/gdynamicDigitization/gdynamicdigitization.h>

#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <vector>

/**
 * \file eventDispenser.h
 * \brief Declares the EventDispenser class.
 */

/**
 * \class EventDispenser
 * \brief Distributes events among run numbers and coordinates dynamic digitization initialization.
 *
 * \details
 * EventDispenser provides a small orchestration layer that:
 * - Interprets user configuration from GOptions (number of events, run selection, optional weight file).
 * - Computes a run-to-event allocation (runEvents).
 * - Iterates over the run allocation and, for each run:
 *   - Invokes all available GDynamicDigitization routines to load run-dependent constants and
 *     translation tables.
 *   - Issues Geant4 commands to execute the requested number of events for that run.
 *
 * \note
 * The *run number* here refers to the simulation conditions (calibration/constants/translation-table
 * selection), not the internal Geant4 run ID.
 */
class EventDispenser : public GBase<EventDispenser>
{
public:
	/**
	 * \brief Constructs an EventDispenser and prepares the run event distribution.
	 *
	 * \details
	 * During construction, the object reads configuration from \c gopt and initializes its internal
	 * maps and lists. Depending on the configuration, it will either:
	 * - Assign all events to a single run number; or
	 * - Load run weights from a text file and derive a per-run event distribution.
	 *
	 * \param gopt Parsed module configuration (e.g. run number, number of events, optional weight file).
	 * \param gdynamicDigitizationMap Map of dynamic digitization routines used to load run-dependent
	 *        constants and translation tables.
	 * \param analysisAccumulator GUI-only Analyzer service; null in batch mode.
	 */
	EventDispenser(const std::shared_ptr<GOptions>&                                 gopt,
	               const std::shared_ptr<const gdynamicdigitization::dRoutinesMap>& gdynamicDigitizationMap,
	               std::shared_ptr<GAnalysisAccumulator> analysisAccumulator = nullptr);

private:
	/**
	 * \name Configuration extracted from GOptions
	 * @{
	 */

	/// Total number of events requested by the user (option \c -n).
	int neventsToProcess;

	/// Run number requested by the user (option \c -run) when not using a run-weight file.
	int userRunno;

	/// True when the configured viewer is TOOLSSG_OFFSCREEN; triggers screenshot commands after BeamOn.
	bool offscreen_screenshots = false;

	/// Most recently processed run number. Used to detect run changes and reload run-dependent data.
	int currentRunno = -1;

	/** @} */

	/**
	 * \name Run distribution state
	 * @{
	 */

	/// Weight assigned to each run number (as read from the run-weight file).
	std::map<int, double> runWeights;

	/// Final number of events assigned to each run number (derived from weights or single-run mode).
	std::map<int, int> runEvents;

	/// Ordered list of run numbers loaded from the run-weight file (used for indexed access).
	std::vector<int> listOfRuns;

	/// Index into \ref listOfRuns used by \ref getCurrentRun "getCurrentRun()".
	int currentRunIndex{};

	/// Wall-clock time when the first \c /run/beamOn was issued; unset until then.
	std::optional<std::chrono::steady_clock::time_point> beamOnTime;

	/** @} */

	/**
	 * \brief Derives the run-to-event allocation from the runWeights map.
	 *
	 * \details
	 * This method uses stochastic sampling: for each event, it draws a uniform random number and
	 * selects the run whose cumulative weight interval contains that draw. The resulting counts
	 * are stored in \ref runEvents.
	 *
	 * \note
	 * This is an internal implementation detail; callers interact with the results through
	 * \ref getRunEvents "getRunEvents()".
	 *
	 * \param nevents_to_process Total number of events that need to be distributed.
	 */
	void distributeEvents(int nevents_to_process);

	/**
	 * \brief Advances the internal run index by one.
	 *
	 * \details
	 * This helper is used only when consuming \ref listOfRuns via \ref getCurrentRun "getCurrentRun()".
	 */
	void setNextRun() { currentRunIndex++; }

	/**
	 * \brief Map of shared pointers to GDynamicDigitization routines.
	 *
	 * \details
	 * The map is owned elsewhere (typically a global or higher-level orchestrator). EventDispenser
	 * keeps a shared pointer to the map so it can:
	 * - iterate the available routines, and
	 * - call their initialization hooks for each run number.
	 */
	std::shared_ptr<const gdynamicdigitization::dRoutinesMap> gDigitizationMap;

	/** \brief GUI-only service that communicates the simulation run number to worker shards. */
	std::shared_ptr<GAnalysisAccumulator> analysisAccumulator;

public:
	/**
	 * \brief Returns the computed run-to-event allocation.
	 *
	 * \details
	 * The allocation is prepared during construction (or after \ref setNumberOfEvents "setNumberOfEvents()"
	 * is called). Keys are run numbers and values are the number of events assigned to that run.
	 *
	 * \return A copy of the run-to-event allocation map.
	 */
	std::map<int, int> getRunEvents() { return runEvents; }

	/**
	 * \brief Returns the current run number from the internally stored run list.
	 *
	 * \details
	 * This accessor is meaningful when a run-weight file was used and listOfRuns contains
	 * the run sequence. The index is currentRunIndex.
	 *
	 * \warning
	 * This method assumes  listOfRuns is non-empty and currentRunIndex is within bounds.
	 *
	 * \return The run number at index currentRunIndex.
	 */
	[[nodiscard]] int getCurrentRun() const { return listOfRuns[currentRunIndex]; }

	/**
	 * \brief Computes the total number of events across all runs.
	 *
	 * \details
	 * This is the sum of all values in runEvents. In a correctly initialized object, it should
	 * match the user-requested event count (option \c -n), except for edge cases where the distribution
	 * inputs are inconsistent.
	 *
	 * \return The total number of events assigned across all runs.
	 */
	[[nodiscard]] int getTotalNumberOfEvents() const;

	/**
	 * \brief Processes all runs by initializing digitization routines and dispatching events.
	 *
	 * \details
	 * For each (runNumber, nevents) pair in runEvents, this method:
	 * - If the run differs from the last processed run, iterates all available digitization routines
	 *   and calls their initialization hooks to load run-dependent data.
	 * - Issues a Geant4 command to execute \c nevents events for that run (currently a single
	 *   \c /run/beamOn call per run allocation entry).
	 *
	 * \return Status code (non-zero indicates success in the current implementation).
	 */
	int processEvents();

	/**
	 * \brief Returns whether at least one \c /run/beamOn has been issued.
	 *
	 * \return True once \ref processEvents() has dispatched its first run.
	 */
	[[nodiscard]] bool beamOnIssued() const { return beamOnTime.has_value(); }

	/**
	 * \brief Returns the wall-clock time of the first \c /run/beamOn.
	 *
	 * \warning Only meaningful when \ref beamOnIssued() returns true.
	 *
	 * \return Steady-clock time point captured just before the first BeamOn command.
	 */
	[[nodiscard]] std::chrono::steady_clock::time_point beamOnStartTime() const { return beamOnTime.value(); }

	/**
	 * \brief Sets the total number of events to process in single-run mode.
	 *
	 * \details
	 * This method exists primarily for GUI-driven workflows, where the user may adjust the event count
	 * interactively. The method clears any existing distribution and assigns all events to  userRunno.
	 *
	 * \param nevts Total number of events to assign to userRunno.
	 */
	void setNumberOfEvents(int nevts);

	/**
	 * \brief Force per-run digitization setup to run again on the next event batch.
	 *
	 * GUI geometry reloads can replace digitization routines without changing the run number.
	 * Resetting the cached run keeps \ref processEvents() from skipping constants/TT setup
	 * after a geometry variation change.
	 */
	void resetRunContext();
};
