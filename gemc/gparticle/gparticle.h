#pragma once

/**
 * @defgroup gparticle_module gparticle
 * \brief Single-particle (and multi-particle) generator configuration and shooting utilities.
 *
 * @ingroup gparticle_topics
 */

/**
 * @defgroup gparticle_topics Topics
 * \brief High-level documentation topics for the gparticle module.
 */

/**
 * @defgroup gparticle_options_topic Options and configuration
 * \brief How gparticle is configured through structured options.
 * @ingroup gparticle_topics
 */

/**
 * @defgroup gparticle_examples_topic Examples
 * \brief Example programs demonstrating how to use the gparticle module.
 * @ingroup gparticle_topics
 */

// gemc
#include <gemc/glogging/glogger.h>
#include <gemc/guts/gutilities.h>

// geant4
#include "G4ThreeVector.hh"
#include "G4ParticleGun.hh"

// c++
#include <string>
#include <vector>

/**
 * \brief Runtime values for one generated primary particle.
 *
 * These values are filled when \ref Gparticle::shootParticle runs and reflect
 * the actual randomized particle passed to Geant4.
 */
struct GparticleRuntimeRecord
{
	std::string name;
	int         pid = 0;
	int         type = 1;
	double      p = 0;
	double      theta = 0;
	double      phi = 0;
	G4ThreeVector vertex;
};


/**
 * \file gparticle.h
 * \brief Definition of the \ref Gparticle class used by the gparticle module.
 *
 * @ingroup gparticle_module
 */

/**
 * @class Gparticle
 * \brief Lightweight particle specification and primary vertex shooter.
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
	 * \brief Constructs a particle configuration from pre-converted G4-unit values.
	 *
	 * All numeric parameters must already be in Geant4 internal units (MeV for
	 * momentum, radians for angles, mm for lengths) as returned by
	 * \c gutilities::getG4Number().  Callers — typically option parsers or file
	 * readers — are responsible for the conversion.
	 *
	 * The particle PDG id is resolved at construction time by consulting the
	 * \c G4ParticleTable using the provided particle name.
	 *
	 * \param name Particle name as understood by \c G4ParticleTable (e.g. \c "e-").
	 * \param multiplicity Number of particles to shoot per event.
	 * \param p Nominal momentum magnitude in G4 internal units (MeV).
	 * \param delta_p Spread parameter for momentum randomization (same units as @p p).
	 * \param randomMomentumModel Random model name for momentum (e.g. \c "uniform", \c "gaussian").
	 * \param theta Nominal polar angle in G4 internal units (radians).
	 * \param delta_theta Spread parameter for theta (same units as @p theta).
	 * \param thetaModel Random model name for theta (e.g. \c "uniform", \c "cosine", \c "gaussian").
	 * \param phi Nominal azimuthal angle in G4 internal units (radians).
	 * \param delta_phi Spread parameter for phi (same units as @p phi).
	 * \param avx Nominal vertex x component in G4 internal units (mm).
	 * \param avy Nominal vertex y component in G4 internal units (mm).
	 * \param avz Nominal vertex z component in G4 internal units (mm).
	 * \param adelta_vx Spread parameter for vertex x (same units as @p avx).
	 * \param adelta_vy Spread parameter for vertex y (same units as @p avy).
	 * \param adelta_vz Spread parameter for vertex z (same units as @p avz).
	 * \param randomVertexModel Random model name for vertex (e.g. \c "uniform", \c "gaussian", \c "sphere").
	 * \param logger Logger used for diagnostics and error reporting.
	 * \param generator_type Generator source type associated with this particle. Inline
	 *        \c -gparticle definitions use the default value \c 1. File-backed particles
	 *        preserve the source-file type field when the format provides one, such as
	 *        the Lund \c type column.
	 */
	Gparticle(const std::string&              name,
			  int                             multiplicity,
			  double                          p,
			  double                          delta_p,
			  const std::string&              randomMomentumModel,
			  double                          theta,
			  double                          delta_theta,
			  const std::string&              thetaModel,
			  double                          phi,
			  double                          delta_phi,
			  double                          avx,
			  double                          avy,
			  double                          avz,
			  double                          adelta_vx,
			  double                          adelta_vy,
			  double                          adelta_vz,
			  const std::string&              randomVertexModel,
			  const std::shared_ptr<GLogger>& logger,
			  int                             generator_type = 1);

	/**
	 * \brief Copying is disabled.
	 *
	 * The class is intended to be managed through \c std::shared_ptr and not copied.
	 */
	Gparticle(const Gparticle&) = delete;

	/**
	 * \brief Copy assignment is disabled.
	 */
	Gparticle& operator=(const Gparticle&) = delete;

	/**
	 * \brief Destructor emits a debug-level lifecycle message.
	 */
	~Gparticle() { log->debug(DESTRUCTOR, "Gparticle"); }

	/**
	 * \brief Shoots this particle configuration into a Geant4 event.
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
	 * \param particleGun The \c G4ParticleGun used to generate the primary vertices.
	 * \param anEvent The \c G4Event that receives the generated primary vertices.
	 */
	void shootParticle(G4ParticleGun* particleGun, G4Event* anEvent);

	/**
	 * \brief Creates a minimal default particle configuration.
	 *
	 * This helper returns an electron with:
	 * - momentum 1 GeV, no spread
	 * - angles 0 deg, no spread
	 * - vertex at (0,0,0) cm, no spread
	 * - uniform random models where relevant
	 *
	 * \param log Logger to associate with the constructed particle.
	 * \return A shared pointer to the default \ref Gparticle configuration.
	 */
	static std::shared_ptr<Gparticle> create_default_gparticle(const std::shared_ptr<GLogger>& log) {
		return std::make_shared<Gparticle>(
										   "e-",
										   1,
										   gutilities::getG4Number("1*GeV"),
										   0,
										   "uniform",
										   0,
										   0,
										   "uniform",
										   0,
										   0,
										   0,
										   0,
										   0,
										   0,
										   0,
										   0,
										   "uniform",
										   log
										  );
	}

	/**
	 * \brief Returns the particle name stored in this generator definition.
	 *
	 * \return Particle name used for Geant4 lookup and generated-particle output.
	 */
	[[nodiscard]] const std::string& getName() const { return name; }

	/**
	 * \brief Returns the resolved PDG particle id.
	 *
	 * \return PDG id resolved from \c G4ParticleTable during construction.
	 */
	[[nodiscard]] int getPid() const { return pid; }

	/**
	 * \brief Returns how many copies are generated per event.
	 *
	 * \return Configured particle multiplicity.
	 */
	[[nodiscard]] int getMultiplicity() const { return multiplicity; }

	/**
	 * \brief Returns the nominal momentum magnitude.
	 *
	 * \return Momentum after unit conversion to GEMC internal units.
	 */
	[[nodiscard]] double getMomentum() const { return p; }

	/**
	 * \brief Returns the nominal polar angle.
	 *
	 * \return Theta after unit conversion to GEMC internal angular units.
	 */
	[[nodiscard]] double getTheta() const { return theta; }

	/**
	 * \brief Returns the nominal azimuthal angle.
	 *
	 * \return Phi after unit conversion to GEMC internal angular units.
	 */
	[[nodiscard]] double getPhi() const { return phi; }

	/**
	 * \brief Returns the nominal vertex position.
	 *
	 * \return Vertex after unit conversion to GEMC internal length units.
	 */
	[[nodiscard]] const G4ThreeVector& getVertex() const { return v; }

	/** \brief Returns the momentum spread parameter (GEMC internal units, MeV). */
	[[nodiscard]] double getDeltaMomentum() const { return delta_p; }

	/** \brief Returns the momentum randomization model as a string token. */
	[[nodiscard]] std::string getMomentumModel() const {
		return gutilities::to_string(randomMomentumModel);
	}

	/** \brief Returns the polar-angle spread parameter (GEMC internal units, radians). */
	[[nodiscard]] double getDeltaTheta() const { return delta_theta; }

	/** \brief Returns the theta randomization model as a string token. */
	[[nodiscard]] std::string getThetaModel() const {
		return gutilities::to_string(randomThetaModel);
	}

	/** \brief Returns the azimuthal-angle spread parameter (GEMC internal units, radians). */
	[[nodiscard]] double getDeltaPhi() const { return delta_phi; }

	/** \brief Returns the vertex spread vector (GEMC internal units, mm). */
	[[nodiscard]] const G4ThreeVector& getDeltaVertex() const { return delta_v; }

	/** \brief Returns the vertex randomization model as a string token. */
	[[nodiscard]] std::string getVertexModel() const {
		return gutilities::to_string(randomVertexModel);
	}

	/**
	 * \brief Returns the generator source type.
	 *
	 * For inline \c -gparticle entries this is normally \c 1. For file-backed
	 * entries this preserves the source format's type field; in Lund files,
	 * type \c 1 is the subset propagated to Geant4 and included in the
	 * \c generated_tracked output bank.
	 *
	 * \return Generator source type.
	 */
	[[nodiscard]] int getGeneratorType() const { return generator_type; }

	/**
	 * \brief Returns the runtime records from the most recent shoot.
	 *
	 * One record is stored for each generated primary, so a particle with
	 * multiplicity N contributes N runtime records.
	 */
	[[nodiscard]] const std::vector<GparticleRuntimeRecord>& getRuntimeRecords() const {
		return runtimeRecords;
	}

	// ---- Setters (used by the GUI to reflect user edits in real time) ----
	void setName(const std::string& n)          { name = n; pid = get_pdg_id(); }
	void setMultiplicity(int m)                 { multiplicity = m; }
	void setMomentum(double p_mev)              { p = p_mev; }
	void setDeltaMomentum(double dp)            { delta_p = dp; }
	void setMomentumModel(const std::string& s) { randomMomentumModel = gutilities::stringToRandomModel(s); }
	void setTheta(double t_rad)                 { theta = t_rad; }
	void setDeltaTheta(double dt)               { delta_theta = dt; }
	void setThetaModel(const std::string& s)    { randomThetaModel = gutilities::stringToRandomModel(s); }
	void setPhi(double ph_rad)                  { phi = ph_rad; }
	void setDeltaPhi(double dp)                 { delta_phi = dp; }
	void setVertexX(double x)                   { v.setX(x); }
	void setVertexY(double y)                   { v.setY(y); }
	void setVertexZ(double z)                   { v.setZ(z); }
	void setDeltaVertexX(double x)              { delta_v.setX(x); }
	void setDeltaVertexY(double y)              { delta_v.setY(y); }
	void setDeltaVertexZ(double z)              { delta_v.setZ(z); }
	void setVertexModel(const std::string& s)   { randomVertexModel = gutilities::stringToRandomModel(s); }

private:
	/** \brief Particle name used to look up the definition in \c G4ParticleTable. */
	std::string name;

	/** \brief Generator source type, for example the Lund particle type column. */
	int generator_type;

	/** \brief Resolved PDG encoding for the particle. */
	int pid;

	/** \brief Number of copies of this particle shot per event. */
	int multiplicity;

	/** \brief Nominal momentum magnitude (internal numeric value after unit conversion). */
	double p;

	/** \brief Spread parameter for momentum randomization (same internal unit as \ref p). */
	double delta_p;

	/** \brief Randomization model used for momentum. */
	gutilities::randomModel randomMomentumModel;

	/** \brief Nominal polar angle (internal numeric value after unit conversion). */
	double theta;

	/** \brief Spread parameter for theta randomization (same internal unit as \ref theta). */
	double delta_theta;

	/** \brief Randomization model used for theta. */
	gutilities::randomModel randomThetaModel;

	/** \brief Nominal azimuthal angle (internal numeric value after unit conversion). */
	double phi;

	/** \brief Spread parameter for phi randomization (same internal unit as \ref phi). */
	double delta_phi;

	/** \brief Nominal vertex position (internal numeric value after unit conversion). */
	G4ThreeVector v;

	/** \brief Spread parameters for vertex components (internal numeric value after unit conversion). */
	G4ThreeVector delta_v;

	/** \brief Randomization model used for the vertex. */
	gutilities::randomModel randomVertexModel;

	/** \brief Logger used for diagnostics and error reporting. */
	std::shared_ptr<GLogger> log;

	/** \brief Actual randomized particles generated by the most recent shoot. */
	std::vector<GparticleRuntimeRecord> runtimeRecords;

	/** \brief Returns the particle mass by consulting \c G4ParticleTable. */
	[[nodiscard]] double get_mass() const;

	/** \brief Stream insertion operator used for pretty-printing configuration summaries. */
	friend std::ostream& operator<<(std::ostream& os, const Gparticle& gp);

	/**
	 * \brief Stream insertion operator overload for shared pointers.
	 *
	 * \param os Output stream.
	 * \param ptr Pointer to a \ref Gparticle; prints \c "<null Gparticle>" if null.
	 * \return Output stream.
	 */
	friend std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Gparticle>& ptr) {
		if (ptr) return os << *ptr;
		else return os << "<null Gparticle>";
	}

	/**
	 * \brief Randomizes a numeric parameter around a center using the selected model.
	 *
	 * \param center Central value.
	 * \param delta Spread parameter.
	 * \param model Randomization model selector.
	 * \return Randomized value (in the same units as @p center).
	 */
	[[nodiscard]] double randomizeNumberFromSigmaWithModel(double                  center,
														   double                  delta,
														   gutilities::randomModel model) const;

	/** \brief Computes (and randomizes) the momentum magnitude for this particle. */
	double calculateMomentum();

	/**
	 * \brief Converts momentum magnitude and mass into kinetic energy.
	 * \param mass Particle mass (internal units).
	 * \return Kinetic energy (internal units).
	 */
	double calculateKinEnergy(double mass);

	/** \brief Computes the beam direction unit vector from randomized theta and phi values. */
	G4ThreeVector calculateBeamDirection(double thetaRad, double phiRad);

	/** \brief Computes the randomized vertex position according to the configured model. */
	G4ThreeVector calculateVertex();

	/** \brief Resolves and returns the PDG encoding for the configured particle name. */
	int get_pdg_id();


	// assigned at shootParticle for each multiplicity
	double        kinenergy;
	G4ThreeVector beamDirection;
	G4ThreeVector vertex;

	void setRunTimeQuantities(double ke, G4ThreeVector bd, G4ThreeVector v) {
		kinenergy     = ke;
		beamDirection = bd;
		vertex        = v;
	}
};

/**
 * \brief Shared pointer type used for \ref Gparticle instances.
 *
 * @ingroup gparticle_module
 */
using GparticlePtr = std::shared_ptr<Gparticle>;
