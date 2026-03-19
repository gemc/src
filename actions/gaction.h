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
 * \file gaction.h
 * \brief Declares GAction, the Geant4 action-initialization entry point for the GEMC actions module.
 *
 * GAction is the module-level class responsible for registering the Geant4 user
 * actions used by GEMC. It derives from \c G4VUserActionInitialization and is
 * queried by the Geant4 run manager to create the action objects appropriate for
 * worker threads, sequential execution, and the master thread.
 *
 * @ingroup gactions_module
 */

constexpr const char* GACTION_LOGGER = "gaction";

/**
 * \brief Namespace containing helpers related to action-subsystem configuration.
 *
 * The functions in this namespace are intended to support module setup by providing
 * a single place where all action-related option definitions can be collected.
 *
 * @ingroup gactions_module
 */
namespace gaction {

/**
 * \brief Builds the aggregate option set required by the actions subsystem.
 *
 * This helper creates a logger-scoped GOptions object for the action module and
 * then merges into it the option definitions contributed by the event action,
 * run action, primary generator action, and run container helpers.
 *
 * The returned object is meant to be merged into the wider application configuration
 * before constructing GAction.
 *
 * \return A GOptions object containing the union of all action-related option definitions.
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
 * \brief Registers GEMC user actions for worker threads, sequential execution, and the master thread.
 *
 * Geant4 uses an action-initialization object derived from \c G4VUserActionInitialization
 * to obtain the user actions that will participate in a simulation.
 *
 * In this module, GAction applies that lifecycle as follows:
 * - \ref GAction::Build "Build()" is invoked for worker threads and also for sequential mode.
 *   It registers:
 *   - GPrimaryGeneratorAction
 *   - GRunAction
 *   - GEventAction
 * - \ref GAction::BuildForMaster "BuildForMaster()" is invoked for the master thread.
 *   It registers:
 *   - GRunAction only
 *
 * This split mirrors the Geant4 execution model:
 * - The master thread coordinates the run and can execute run-level hooks.
 * - Worker threads execute event processing and therefore require generator
 *   and event actions in addition to the run action.
 *
 * Internally, this class stores the shared configuration object and the shared
 * digitization-routine map so that every action created by this initializer
 * receives a consistent view of the runtime configuration.
 *
 * @ingroup gactions_module
 */
class GAction : public GBase<GAction>, public G4VUserActionInitialization {
public:
	/**
	 * \brief Constructs the action initializer used by the Geant4 run manager.
	 *
	 * \param gopts Shared configuration object used by all actions constructed by this initializer.
	 * \param digi_map Shared map associating collection names with digitization routines.
	 */
	explicit GAction(std::shared_ptr<GOptions> gopts,
	                 std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);

	~GAction() override = default;

	GAction(const GAction&)            = delete;
	GAction& operator=(const GAction&) = delete;
	GAction(GAction&&)                 = delete;
	GAction& operator=(GAction&&)      = delete;

	/**
	 * \brief Registers the user actions required by worker threads and sequential execution.
	 *
	 * This method is called by Geant4 when action objects must be created for an
	 * execution context that processes events.
	 *
	 * The registration order used here is:
	 * - GPrimaryGeneratorAction, to generate event primaries.
	 * - GRunAction, to manage run begin/end hooks and create GRun.
	 * - GEventAction, to process hit collections, digitize them, and publish event data.
	 *
	 * The event action receives the run-action instance created in this method so it
	 * can access thread-local run services such as digitization-routine lookup and
	 * streamer access.
	 */
	void Build() const override;

	/**
	 * \brief Registers the user actions required by the master thread.
	 *
	 * In multithreaded execution, the master thread does not process individual events.
	 * For that reason, only GRunAction is registered here. That action handles the
	 * master-side run lifecycle, including merged run-data publication when needed.
	 */
	void BuildForMaster() const override;

private:
	/**
	 * \brief Shared configuration used when constructing all action objects.
	 *
	 * This pointer is stored so both \ref GAction::Build "Build()" and
	 * \ref GAction::BuildForMaster "BuildForMaster()" create actions using the same
	 * runtime configuration object.
	 */
	std::shared_ptr<GOptions> goptions;

	/**
	 * \brief Shared digitization-routine map passed to run and event processing actions.
	 *
	 * The map associates collection names with the routines that know how to transform
	 * raw hit information into GEMC digitized and truth-level payload.
	 */
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;
};