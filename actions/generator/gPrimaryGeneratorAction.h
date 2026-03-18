#pragma once

#include <memory>
#include <vector>

// gemc
#include "goptions.h"
#include "gbase.h"
#include "gparticle_options.h"

// geant4
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"

/**
 * @file gPrimaryGeneratorAction.h
 * @brief Declares GPrimaryGeneratorAction, the primary-particle generation action for the GEMC actions module.
 *
 * @ingroup gactions_module
 */

constexpr const char* GPRIMARYGENERATORACTION_LOGGER = "generator";

/**
 * @brief Namespace containing helpers related to primary-generator configuration.
 *
 * @ingroup gactions_module
 */
namespace gprimaryaction {

/**
 * @brief Returns the options associated with the primary-generator action scope.
 *
 * In the current implementation this helper returns a logger-scoped GOptions object
 * without adding dedicated primary-generator switches directly in this file.
 * Generator behavior is instead mainly driven by the particle definitions loaded
 * through the generator support code.
 *
 * @return A GOptions object scoped to the primary-generator logger name.
 */
inline GOptions defineOptions() {
	return GOptions(GPRIMARYGENERATORACTION_LOGGER);
}

} // namespace gprimaryaction


/**
 * @class GPrimaryGeneratorAction
 * @brief Generates the primary vertices for each Geant4 event.
 *
 * This class is the GEMC implementation of the Geant4 primary-generator action.
 * It owns a \c G4ParticleGun and a list of configured Gparticle objects.
 *
 * For every event, \ref GPrimaryGeneratorAction::GeneratePrimaries "GeneratePrimaries()"
 * iterates over the configured particle definitions and delegates the actual shooting
 * operation to each Gparticle instance. This allows the generator configuration to be
 * data-driven while still using the Geant4 particle-gun mechanism underneath.
 *
 * If no particle definitions are found in the configuration, the constructor creates
 * a default particle so the simulation still has a valid primary source.
 *
 * @ingroup gactions_module
 */
class GPrimaryGeneratorAction : public GBase<GPrimaryGeneratorAction>, public G4VUserPrimaryGeneratorAction {
public:
	/**
	 * @brief Constructs the primary-generator action.
	 *
	 * Construction performs the following steps:
	 * - allocate the \c G4ParticleGun instance used for emission;
	 * - read the configured particle definitions through gparticle support helpers;
	 * - create a default particle if no explicit particle definition is available.
	 *
	 * @param gopts Shared configuration object used to retrieve generator settings and logging controls.
	 */
	explicit GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts);

	~GPrimaryGeneratorAction() override = default;

	GPrimaryGeneratorAction(const GPrimaryGeneratorAction&)            = delete;
	GPrimaryGeneratorAction& operator=(const GPrimaryGeneratorAction&) = delete;
	GPrimaryGeneratorAction(GPrimaryGeneratorAction&&)                 = delete;
	GPrimaryGeneratorAction& operator=(GPrimaryGeneratorAction&&)      = delete;

	/**
	 * @brief Generates the primary content for one event.
	 *
	 * The method loops over all configured Gparticle objects. For each non-null entry,
	 * it asks the particle object to configure the internal \c G4ParticleGun and to
	 * inject the resulting primary information into the supplied event.
	 *
	 * This design allows multiple configured particle definitions to contribute to a
	 * single Geant4 event.
	 *
	 * @param event The event that will receive the generated primary vertices and particles.
	 */
	void GeneratePrimaries(G4Event* event) override;

private:
	/**
	 * @brief Particle-gun instance used to materialize configured primaries into the event.
	 *
	 * The gun is owned exclusively by this class and reused across events. Individual
	 * Gparticle objects configure it before shooting their contribution to the event.
	 */
	std::unique_ptr<G4ParticleGun> gparticleGun;

	/**
	 * @brief List of configured particle definitions used during event generation.
	 *
	 * Each entry represents one generator-side particle description. During event
	 * generation the list is traversed in order, and each non-null entry is given
	 * the opportunity to contribute primary content to the event.
	 */
	std::vector<GparticlePtr> gparticles;
};