#pragma once

// geant4
#include "G4Run.hh"

// gemc
#include "goptions.h"
#include "gdynamicdigitization.h"

/**
 * \file gRun.h
 * \brief Declares GRun, the thread-local run container used by the GEMC actions module.
 *
 * @ingroup gactions_module
 */

constexpr const char* GRUN_LOGGER = "grun";

using GHitsCollection = G4THitsCollection<GHit>;

/**
 * \brief Namespace containing helpers related to the run-container scope.
 *
 * @ingroup gactions_module
 */
namespace grun {
/**
 * \brief Returns the options associated with the run-container logger scope.
 *
 * \return A GOptions object scoped to the run-container logger name.
 */
inline GOptions defineOptions() { return GOptions(GRUN_LOGGER); }
} // namespace grun


/**
 * @class GRun
 * \brief Thread-local run object created by the GEMC run action.
 *
 * In Geant4, a run represents a sequence of events managed under one BeamOn() request.
 * In multithreaded execution, each worker thread owns its own run object, and Geant4
 * may later merge worker-side run information into a master-level view.
 *
 * In this module, GRun acts as the run object instantiated through
 * the private method \c GenerateRun(). Its current role is intentionally small:
 * it stores access to the shared digitization-routine map so the run-level execution
 * context has a consistent view of the digitization configuration throughout the run.
 *
 * Even though the class is lightweight, it is the natural extension point for future
 * run-scoped services and state that should live for the duration of a thread-local run.
 *
 * @ingroup gactions_module
 */
class GRun : public GBase<GRun>, public G4Run {
public:
	/**
	 * \brief Constructs the run object for the current execution thread.
	 *
	 * \param gopts Shared configuration object used for logging and any run-scoped behavior.
	 * \param digi_map Shared digitization-routine map associated with this run context.
	 */
	explicit GRun(std::shared_ptr<GOptions> gopts,
	              std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);

	~GRun() override = default;

	GRun(const GRun&)            = delete;
	GRun& operator=(const GRun&) = delete;
	GRun(GRun&&)                 = delete;
	GRun& operator=(GRun&&)      = delete;

private:
	/**
	 * \brief Shared digitization-routine map retained for the lifetime of the run object.
	 *
	 * The map associates collection names with the routines used to transform raw hit
	 * data into GEMC digitized and truth-level payload. Storing it here ensures the
	 * run object and the components operating within the run observe the same configuration.
	 */
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;
};