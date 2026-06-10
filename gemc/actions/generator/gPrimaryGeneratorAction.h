#pragma once

#include <memory>
#include <vector>

// gemc
#include <gemc/goptions/goptions.h>
#include <gemc/gbase/gbase.h>
#include <gemc/gparticle/gparticle_options.h>
#include <gemc/gparticle/gparticle_reader.h>

// geant4
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"

/**
 * \file gPrimaryGeneratorAction.h
 * \brief Declares GPrimaryGeneratorAction, the primary-particle generation action for the GEMC actions module.
 *
 * @ingroup gactions_module
 */

constexpr const char* GPRIMARYGENERATORACTION_LOGGER = "generator";

/**
 * \brief Namespace containing helpers related to primary-generator configuration.
 *
 * @ingroup gactions_module
 */
namespace gprimaryaction {

/**
 * \brief Returns the options associated with the primary-generator action scope.
 *
 * In the current implementation this helper returns a logger-scoped GOptions object
 * without adding dedicated primary-generator switches directly in this file.
 * Generator behavior is instead mainly driven by the particle definitions loaded
 * through the generator support code.
 *
 * \return A GOptions object scoped to the primary-generator logger name.
 */
inline GOptions defineOptions() {
	return GOptions(GPRIMARYGENERATORACTION_LOGGER);
}

} // namespace gprimaryaction


/**
 * @class GPrimaryGeneratorAction
 * \brief Generates the primary vertices for each Geant4 event.
 *
 * This class is the GEMC implementation of the Geant4 primary-generator action.
 * It owns a \c G4ParticleGun and a list of configured Gparticle objects.
 * Inline particles from \c -gparticle are generated for every event, while
 * \c -gparticlefile sources are indexed by Geant4 event id.
 *
 * For every event, \ref GPrimaryGeneratorAction::GeneratePrimaries "GeneratePrimaries()"
 * iterates over the configured particle definitions and delegates the actual shooting
 * operation to each Gparticle instance. This allows the generator configuration to be
 * data-driven while still using the Geant4 particle-gun mechanism underneath.
 *
 * If no particle definitions are found in the configuration, the constructor creates
 * a default particle so the simulation still has a valid primary source.
 *
 * The class also exposes thread-local generated-particle snapshots for event
 * output. The \c generated snapshot includes inline particles and all parsed
 * file rows. The \c generated_tracked snapshot includes inline particles and
 * only the file rows propagated in Geant4.
 *
 * @ingroup gactions_module
 */
class GPrimaryGeneratorAction : public GBase<GPrimaryGeneratorAction>, public G4VUserPrimaryGeneratorAction {
public:
	/**
	 * \brief Constructs the primary-generator action.
	 *
	 * Construction performs the following steps:
	 * - allocate the \c G4ParticleGun instance used for emission;
	 * - read the configured particle definitions through gparticle support helpers;
	 * - create a default particle if no explicit particle definition is available.
	 *
	 * \param gopts Shared configuration object used to retrieve generator settings and logging controls.
	 */
	explicit GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts);

	/** \brief Construct with a pre-populated shared particle list (used by GAction so all
	 *         worker-thread instances share the same GparticlePtr objects as the GUI). */
	GPrimaryGeneratorAction(std::shared_ptr<GOptions> gopts,
	                        std::shared_ptr<std::vector<GparticlePtr>> particles);

	~GPrimaryGeneratorAction() override = default;

	GPrimaryGeneratorAction(const GPrimaryGeneratorAction&)            = delete;
	GPrimaryGeneratorAction& operator=(const GPrimaryGeneratorAction&) = delete;
	GPrimaryGeneratorAction(GPrimaryGeneratorAction&&)                 = delete;
	GPrimaryGeneratorAction& operator=(GPrimaryGeneratorAction&&)      = delete;

	/**
	 * \brief Generates the primary content for one event.
	 *
	 * The method loops over all configured Gparticle objects. For each non-null entry,
	 * it asks the particle object to configure the internal \c G4ParticleGun and to
	 * inject the resulting primary information into the supplied event.
	 *
	 * This design allows multiple configured particle definitions to contribute to a
	 * single Geant4 event.
	 *
	 * \param event The event that will receive the generated primary vertices and particles.
	 */
	void GeneratePrimaries(G4Event* event) override;

	/**
	 * \brief Returns the current event's Geant4-propagated generated particles.
	 *
	 * \return Thread-local generated particles for the active event.
	 */
	static const GParticleEvent& currentGeneratedParticles();

	/**
	 * \brief Returns the current event's Geant4-tracked generated particles.
	 *
	 * This view contains inline particles and file-backed particles propagated
	 * in Geant4.
	 *
	 * \return Thread-local tracked generated particles for the active event.
	 */
	static const GParticleEvent& currentGeneratedTrackedParticles();

	/**
	 * \brief Returns the current event's full generated-particle records.
	 *
	 * This is the source for the \c generated output bank. It includes inline
	 * particles and every parsed file-backed particle row.
	 *
	 * \return Thread-local generated-particle records for the active event.
	 */
	static const GParticleRecordEvent& currentGeneratedParticleRecords();

	/**
	 * \brief Returns the current event's Geant4-tracked generated-particle records.
	 *
	 * This is the source for the \c generated_tracked output bank. It includes
	 * inline particles and only file-backed particles propagated in Geant4.
	 *
	 * \return Thread-local tracked generated-particle records for the active event.
	 */
	static const GParticleRecordEvent& currentGeneratedTrackedParticleRecords();

private:
	/**
	 * \brief Particle-gun instance used to materialize configured primaries into the event.
	 *
	 * The gun is owned exclusively by this class and reused across events. Individual
	 * Gparticle objects configure it before shooting their contribution to the event.
	 */
	std::unique_ptr<G4ParticleGun> gparticleGun;

	/**
	 * \brief List of configured particle definitions used during event generation.
	 *
	 * Each entry represents one generator-side particle description. During event
	 * generation the list is traversed in order, and each non-null entry is given
	 * the opportunity to contribute primary content to the event.
	 */
	std::shared_ptr<std::vector<GparticlePtr>> gparticles;

	/**
	 * \brief File-backed particle events, indexed by Geant4 event id.
	 *
	 * These records are kept separate from \ref gparticles so worker threads do not
	 * replay the full file on every event. Each generated event consumes only the
	 * matching file event record.
	 */
	GParticleEvents gparticleFileEvents;

	/**
	 * \brief File-backed generated-particle records, indexed by Geant4 event id.
	 *
	 * This record view preserves all parsed file particles for the
	 * \c generated output bank, including rows that are not propagated in Geant4.
	 */
	GParticleRecordEvents allGparticleFileRecordEvents;

	/// \brief Thread-local \ref GParticleEvent snapshot for the current event.
	static thread_local GParticleEvent current_generated_particles;

	/// \brief Thread-local tracked \ref GParticleEvent snapshot for the current event.
	static thread_local GParticleEvent current_generated_tracked_particles;

	/// \brief Thread-local full generated-particle record snapshot for the current event.
	static thread_local GParticleRecordEvent current_generated_particle_records;

	/// \brief Thread-local tracked generated-particle record snapshot for the current event.
	static thread_local GParticleRecordEvent current_generated_tracked_particle_records;
};
