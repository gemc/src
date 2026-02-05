#pragma once

// geant4
#include "G4Run.hh"

// gemc
#include "goptions.h"
#include "gdynamicdigitization.h"

/**
 * @file gRun.h
 * @brief Declares GRun, the per-thread run container.
 *
 * @ingroup gactions_module
 */

constexpr const char* GRUN_LOGGER = "grun";

using GHitsCollection = G4THitsCollection<GHit>;

/**
 * @brief Namespace collecting helpers for the run container.
 *
 * @ingroup gactions_module
 */
namespace grun {
/**
 * @brief Returns the options associated with the run container scope.
 *
 * @return A GOptions instance for the run container logger scope.
 */
inline GOptions defineOptions() { return GOptions(GRUN_LOGGER); }
} // namespace grun


/**
 * @class GRun
 * @brief Thread-local run object created for each Geant4 run.
 *
 * In Geant4, a run is a sequence of events started by the run manager BeamOn().
 * In multithreaded mode, each worker thread creates its own run object; Geant4 may
 * later merge worker runs into a global run.
 *
 * GEMC uses this run object as the place where run-level services can be attached.
 * In particular, the digitization routines map is stored so that event-level logic
 * can reference consistent digitization behavior for the lifetime of the run.
 *
 * Creation:
 * - Instances are created by the run action via GRunAction::GenerateRun().
 *
 * @ingroup gactions_module
 */
class GRun : public GBase<GRun>, public G4Run {
public:
	/**
	 * @brief Constructs the run object for the current thread.
	 *
	 * @param gopts Shared configuration used by this run for logging and run-level behavior.
	 * @param digi_map Shared digitization routines map used throughout the run.
	 */
	GRun(std::shared_ptr<GOptions> gopts, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);

private:
	/**
	 * @brief Digitization routines map used to digitize hit collections during the run.
	 *
	 * The map is provided by higher-level initialization code and is expected to
	 * remain valid for the run lifetime.
	 */
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;
};
