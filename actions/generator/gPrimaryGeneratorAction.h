#pragma once

// gemc
#include "goptions.h"
#include "gbase.h"
#include "gparticle_options.h"

// geant4
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"

/**
 * @file gPrimaryGeneratorAction.h
 * @brief Declares GPrimaryGeneratorAction, the primary particle generation action.
 *
 * @ingroup gactions_module
 */

constexpr const char* GPRIMARYGENERATORACTION_LOGGER = "generator";

/**
 * @brief Namespace collecting helpers for the primary generator action.
 *
 * @ingroup gactions_module
 */
namespace gprimaryaction {
/**
 * @brief Returns the options associated with the primary generator action.
 *
 * This module currently returns an empty option set; options may be provided by
 * other generator-related helpers (e.g., gparticle options).
 *
 * @return A GOptions instance for the primary generator action logger scope.
 */
inline GOptions defineOptions() { return GOptions(GPRIMARYGENERATORACTION_LOGGER); }
} // namespace gprimaryaction


/**
 * @class GPrimaryGeneratorAction
 * @brief Generates primary vertices for each event.
 *
 * This action constructs a Geant4 particle gun (\c G4ParticleGun) and a list of
 * configured Gparticle objects. For each event, it iterates over the list and
 * delegates to each Gparticle instance to configure the gun and shoot into the event.
 *
 * If no particles are configured, a default particle is created and used.
 *
 * @ingroup gactions_module
 */
class GPrimaryGeneratorAction : public GBase<GPrimaryGeneratorAction>, public G4VUserPrimaryGeneratorAction {
public:
	/**
	 * @brief Constructs the primary generator action.
	 *
	 * - Allocates the \c G4ParticleGun instance.
	 * - Loads configured particles using gparticle::getGParticles().
	 * - If none are defined, creates a default particle.
	 *
	 * @param gopts Shared configuration object used to retrieve particle definitions and logging settings.
	 */
	GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts);

	/**
	 * @brief Destructor. Releases the internally owned particle gun.
	 */
	~GPrimaryGeneratorAction() override;

	/**
	 * @brief Generates the primaries for the given event.
	 *
	 * The method loops over the configured Gparticle list, logs particle details
	 * at higher verbosity, and invokes the per-particle shooting routine.
	 *
	 * @param event The Geant4 event that will receive the generated primaries.
	 */
	void GeneratePrimaries(G4Event* event) override;

private:
	/**
	 * @brief Geant4 particle gun used as the emission mechanism for all configured particles.
	 *
	 * Ownership: this class owns the pointer and deletes it in the destructor.
	 */
	G4ParticleGun* gparticleGun;

	/**
	 * @brief List of configured particles to be generated for each event.
	 *
	 * Each entry represents an independent particle definition and is asked to
	 * produce one primary (or a set of primaries) via its shooting routine.
	 */
	std::vector<GparticlePtr> gparticles;
};
