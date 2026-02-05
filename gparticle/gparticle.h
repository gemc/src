#pragma once

/**
 * @defgroup gparticle_module gparticle
 * @brief Single-particle (and multi-particle) generator configuration and shooting utilities.
 *
 * @ingroup gparticle_topics
 */

/**
 * @defgroup gparticle_topics Topics
 * @brief High-level documentation topics for the gparticle module.
 */

/**
 * @defgroup gparticle_options_topic Options and configuration
 * @brief How gparticle is configured through structured options.
 * @ingroup gparticle_topics
 */

/**
 * @defgroup gparticle_examples_topic Examples
 * @brief Example programs demonstrating how to use the gparticle module.
 * @ingroup gparticle_topics
 */

// gemc
#include "glogger.h"
#include "gutilities.h"

// geant4
#include "G4ThreeVector.hh"
#include "G4ParticleGun.hh"

// c++
#include <string>


/**
 * @file gparticle.h
 * @brief Definition of the \ref Gparticle class used by the gparticle module.
 *
 * @ingroup gparticle_module
 */

/**
 * @class Gparticle
 * @brief Lightweight particle specification and primary vertex shooter.
 *
 * A \ref Gparticle instance represents a generator-level particle configuration
 * that can be used to produce primary vertices in a \c G4Event through a
 * \c G4ParticleGun.
 *
 * The class stores:
 * - **Identity**: particle name and resolved PDG id
 * - **Multiplicity**: number of copies shot per event
 * - **Kinematics**: momentum magnitude and angular parameters
 * - **Vertex**: position and optional spread/randomization
 * - **Randomization models**: selection of uniform/gaussian/cosine (angles) and sphere (vertex)
 *
 * Configuration is typically created by option parsing utilities and then used
 * during event generation by calling \ref Gparticle::shootParticle "shootParticle()".
 *
 * Logging:
 * - A logger is provided at construction and retained for diagnostics.
 * - Verbosity 2 typically prints a full configuration summary via the stream operator.
 *
 * @ingroup gparticle_module
 */
class Gparticle
{
public:
	/**
	 * @brief Constructs a particle configuration from user-facing parameters.
	 *
	 * This constructor converts user-facing numeric values paired with unit strings
	 * into internal numeric values (in the unit system returned by gutilities helpers),
	 * and converts randomization model strings into gutilities::randomModel values.
	 *
	 * The particle PDG id is resolved at construction time by consulting the
	 * \c G4ParticleTable using the provided particle name.
	 *
	 * @param name Particle name as understood by \c G4ParticleTable (e.g. \c "e-").
	 * @param multiplicity Number of particles to shoot per event.
	 * @param p Nominal momentum magnitude (interpreted together with @p punit).
	 * @param delta_p Spread parameter for momentum randomization (interpreted with @p punit).
	 * @param punit Unit string used to interpret @p p and @p delta_p (e.g. \c "MeV", \c "GeV").
	 * @param randomMomentumModel Random model name for momentum (e.g. \c "uniform", \c "gaussian").
	 * @param theta Nominal polar angle (interpreted together with @p aunit).
	 * @param delta_theta Spread parameter for theta (interpreted with @p aunit).
	 * @param thetaModel Random model name for theta (e.g. \c "uniform", \c "cosine", \c "gaussian").
	 * @param phi Nominal azimuthal angle (interpreted together with @p aunit).
	 * @param delta_phi Spread parameter for phi (interpreted with @p aunit).
	 * @param aunit Unit string used to interpret angles (typically \c "deg" or \c "rad").
	 * @param avx Nominal vertex x component (interpreted together with @p vunit).
	 * @param avy Nominal vertex y component (interpreted together with @p vunit).
	 * @param avz Nominal vertex z component (interpreted together with @p vunit).
	 * @param adelta_vx Spread parameter for vertex x (interpreted with @p vunit).
	 * @param adelta_vy Spread parameter for vertex y (interpreted with @p vunit).
	 * @param adelta_vz Spread parameter for vertex z (interpreted with @p vunit).
	 * @param vunit Unit string used to interpret vertex components (e.g. \c "cm", \c "mm").
	 * @param randomVertexModel Random model name for vertex (e.g. \c "uniform", \c "gaussian", \c "sphere").
	 * @param logger Logger used for diagnostics and error reporting.
	 */
	Gparticle(const std::string&              name,
	          int                             multiplicity,
	          double                          p,
	          double                          delta_p,
	          const std::string&              punit,
	          const std::string&              randomMomentumModel,
	          double                          theta,
	          double                          delta_theta,
	          const std::string&              thetaModel,
	          double                          phi,
	          double                          delta_phi,
	          const std::string&              aunit,
	          double                          avx,
	          double                          avy,
	          double                          avz,
	          double                          adelta_vx,
	          double                          adelta_vy,
	          double                          adelta_vz,
	          const std::string&              vunit,
	          const std::string&              randomVertexModel,
	          const std::shared_ptr<GLogger>& logger);

	/**
	 * @brief Copying is disabled.
	 *
	 * The class is intended to be managed through \c std::shared_ptr and not copied.
	 */
	Gparticle(const Gparticle&) = delete;

	/**
	 * @brief Copy assignment is disabled.
	 */
	Gparticle& operator=(const Gparticle&) = delete;

	/**
	 * @brief Destructor emits a debug-level lifecycle message.
	 */
	~Gparticle() { log->debug(DESTRUCTOR, "Gparticle"); }

	/**
	 * @brief Shoots this particle configuration into a Geant4 event.
	 *
	 * The method resolves the particle definition from the \c G4ParticleTable
	 * (using the stored particle name) and then, for each copy defined by
	 * the configured multiplicity:
	 * - sets the particle kinetic energy based on randomized momentum and mass
	 * - sets the momentum direction based on randomized theta/phi
	 * - sets the vertex position based on the configured vertex model
	 * - calls \c GeneratePrimaryVertex on the provided \c G4ParticleGun
	 *
	 * Error handling:
	 * - If the \c G4ParticleTable is unavailable, an error is logged.
	 * - If the particle is not found, an error is logged.
	 *
	 * @param particleGun The \c G4ParticleGun used to generate the primary vertices.
	 * @param anEvent The \c G4Event that receives the generated primary vertices.
	 */
	void shootParticle(G4ParticleGun* particleGun, G4Event* anEvent);

	/**
	 * @brief Creates a minimal default particle configuration.
	 *
	 * This helper returns an electron with:
	 * - momentum 1 GeV, no spread
	 * - angles 0 deg, no spread
	 * - vertex at (0,0,0) cm, no spread
	 * - uniform random models where relevant
	 *
	 * @param log Logger to associate with the constructed particle.
	 * @return A shared pointer to the default \ref Gparticle configuration.
	 */
	static std::shared_ptr<Gparticle> create_default_gparticle(const std::shared_ptr<GLogger>& log) {
		return std::make_shared<Gparticle>(
			"e-",
			1,
			1,
			0,
			"GeV",
			"uniform",
			0,
			0,
			"uniform",
			0,
			0,
			"deg",
			0,
			0,
			0,
			0,
			0,
			0,
			"cm",
			"uniform",
			log
		);
	}

private:
	/** @brief Particle name used to look up the definition in \c G4ParticleTable. */
	std::string name;

	/** @brief Resolved PDG encoding for the particle. */
	int pid;

	/** @brief Number of copies of this particle shot per event. */
	int multiplicity;

	/** @brief Nominal momentum magnitude (internal numeric value after unit conversion). */
	double p;

	/** @brief Spread parameter for momentum randomization (same internal unit as \ref p). */
	double delta_p;

	/** @brief Randomization model used for momentum. */
	gutilities::randomModel randomMomentumModel;

	/** @brief Nominal polar angle (internal numeric value after unit conversion). */
	double theta;

	/** @brief Spread parameter for theta randomization (same internal unit as \ref theta). */
	double delta_theta;

	/** @brief Randomization model used for theta. */
	gutilities::randomModel randomThetaModel;

	/** @brief Nominal azimuthal angle (internal numeric value after unit conversion). */
	double phi;

	/** @brief Spread parameter for phi randomization (same internal unit as \ref phi). */
	double delta_phi;

	/** @brief Nominal vertex position (internal numeric value after unit conversion). */
	G4ThreeVector v;

	/** @brief Spread parameters for vertex components (internal numeric value after unit conversion). */
	G4ThreeVector delta_v;

	/** @brief Randomization model used for the vertex. */
	gutilities::randomModel randomVertexModel;

	/** @brief Logger used for diagnostics and error reporting. */
	std::shared_ptr<GLogger> log;

	/** @brief Returns the particle mass by consulting \c G4ParticleTable. */
	[[nodiscard]] double get_mass() const;

	/** @brief Stream insertion operator used for pretty-printing configuration summaries. */
	friend std::ostream& operator<<(std::ostream& os, const Gparticle& gp);

	/**
	 * @brief Stream insertion operator overload for shared pointers.
	 *
	 * @param os Output stream.
	 * @param ptr Pointer to a \ref Gparticle; prints \c "<null Gparticle>" if null.
	 * @return Output stream.
	 */
	friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Gparticle>& ptr) {
		if (ptr) return os << *ptr;
		else return os << "<null Gparticle>";
	}

	/**
	 * @brief Randomizes a numeric parameter around a center using the selected model.
	 *
	 * @param center Central value.
	 * @param delta Spread parameter.
	 * @param model Randomization model selector.
	 * @return Randomized value (in the same units as @p center).
	 */
	[[nodiscard]] double randomizeNumberFromSigmaWithModel(double                  center,
	                                                       double                  delta,
	                                                       gutilities::randomModel model) const;

	/** @brief Computes (and randomizes) the momentum magnitude for this particle. */
	double calculateMomentum();

	/**
	 * @brief Converts momentum magnitude and mass into kinetic energy.
	 * @param mass Particle mass (internal units).
	 * @return Kinetic energy (internal units).
	 */
	double calculateKinEnergy(double mass);

	/** @brief Computes the randomized beam direction unit vector. */
	G4ThreeVector calculateBeamDirection();

	/** @brief Computes the randomized vertex position according to the configured model. */
	G4ThreeVector calculateVertex();

	/** @brief Resolves and returns the PDG encoding for the configured particle name. */
	int get_pdg_id();
};

/**
 * @brief Shared pointer type used for \ref Gparticle instances.
 *
 * @ingroup gparticle_module
 */
using GparticlePtr = std::shared_ptr<Gparticle>;
