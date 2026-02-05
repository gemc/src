#pragma once

// geant4
#include "G4VUserActionInitialization.hh"

// gemc
#include "gbase.h"
#include "gdynamicdigitization.h"
#include "gstreamer.h"

// the other actions, so we can define the options all at once
#include "event/gEventAction.h"
#include "run/gRunAction.h"
#include "run/gRun.h"
#include "generator/gPrimaryGeneratorAction.h"

/**
 * @file gaction.h
 * @brief Declares GAction, the Geant4 action-initialization entry point for GEMC.
 *
 * GAction derives from the Geant4 action initialization interface (\c G4VUserActionInitialization)
 * and wires together the run, event, and primary generation actions used by GEMC.
 *
 * @ingroup gactions_module
 */

constexpr const char* GACTION_LOGGER = "gaction";

/**
 * @brief Namespace collecting helpers for the actions subsystem.
 *
 * @ingroup gactions_module
 */
namespace gaction {

/**
 * @brief Builds and returns the complete set of options required by the actions subsystem.
 *
 * This helper is intended to be used by the application/module setup to define all
 * actions-related options in one place.
 *
 * It aggregates:
 * - event action options
 * - run action options
 * - primary generator options
 * - run container options
 *
 * @return A GOptions instance populated with the union of all action-related options.
 */
inline GOptions defineOptions() {
	auto goptions = GOptions(GACTION_LOGGER);
	goptions += geventaction::defineOptions();
	goptions += grunaction::defineOptions();
	goptions += gprimaryaction::defineOptions();
	goptions += grun::defineOptions();
	return goptions;
}
} // namespace gaction


/**
 * @class GAction
 * @brief Registers GEMC user actions for worker and master threads.
 *
 * Geant4 uses an action initialization class (\c G4VUserActionInitialization) to instantiate
 * user action objects. These action objects are typically thread-local:
 *
 * - Build() is invoked for worker threads and also for sequential mode.
 * - BuildForMaster() is invoked for the master thread, and is commonly used to register
 *   only the run action.
 *
 * This class holds:
 * - A shared pointer to GOptions, used by the constructed actions to read runtime configuration.
 * - A shared pointer to the digitization routines map, used by the run and event actions to
 *   digitize hits and publish results.
 *
 * @ingroup gactions_module
 */
class GAction : public GBase<GAction>, public G4VUserActionInitialization {
public:
	/**
	 * @brief Constructs the action initializer.
	 *
	 * @param gopts Shared configuration object used by all actions constructed by this initializer.
	 * @param digi_map Shared map from sensitive detector / hit collection name to digitization routines.
	 */
	GAction(std::shared_ptr<GOptions> gopts, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);

	/**
	 * @brief Registers user actions for worker threads (and sequential mode).
	 *
	 * Expected registrations include:
	 * - The primary generator action.
	 * - The run action.
	 * - The event action.
	 *
	 * The constructed actions receive the shared configuration and digitization map.
	 */
	void Build() const override;

	/**
	 * @brief Registers user actions for the master thread.
	 *
	 * In multithreaded mode, the master thread typically registers only the run action.
	 */
	void BuildForMaster() const override;

private:
	/**
	 * @brief Shared configuration used to construct and configure all action objects.
	 *
	 * This pointer is kept so that Build() and BuildForMaster() can construct the action objects
	 * using the same configuration instance.
	 */
	std::shared_ptr<GOptions> goptions;

	/**
	 * @brief Digitization routines map used by run/event actions to digitize hit collections.
	 *
	 * The map is populated elsewhere (e.g., during sensitive detector and field construction)
	 * and is shared across threads as a read-mostly structure.
	 */
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;
};
