#pragma once

// gparticle
#include "gparticle.h"

// gemc
#include <gemc/gbase/gbase.h>
#include <gemc/gfactory/gdl.h>
#include <gemc/goptions/goptions.h>

// c++
#include <memory>
#include <string>
#include <utility>
#include <vector>

/**
 * \brief Event-local list of Geant4-propagated generator particles.
 *
 * Each entry is a \ref Gparticle that can be shot into a \c G4Event. File
 * readers usually populate this list with only the particles that should be
 * propagated by Geant4, such as Lund rows with \c type == 1.
 */
using GParticleEvent  = std::vector<GparticlePtr>;

/**
 * \brief Sequence of file-backed generated-particle events.
 *
 * The outer vector is indexed by event number; each inner vector contains the
 * particles for that input event that can be represented as \ref Gparticle
 * objects and propagated to Geant4.
 */
using GParticleEvents = std::vector<GParticleEvent>;

/**
 * \brief Immutable generated-particle metadata used for output banks.
 *
 * A record is a serialization-friendly description of a generated particle.
 * Unlike \ref Gparticle, it does not require that the particle name or id can
 * be resolved by \c G4ParticleTable. This allows output banks to preserve all
 * particles found in an input file, including bookkeeping or event-generator
 * rows that are not propagated in Geant4.
 *
 * The kinematic and vertex values are stored after the reader's unit
 * conversion. For Lund input, \c type preserves the Lund particle type column;
 * \c type == 1 identifies rows propagated by Geant4.
 */
struct GParticleRecord
{
	/// Particle name when known, otherwise a source-format identifier such as the numeric pid.
	std::string name;

	/// PDG id or source particle id.
	int pid = 0;

	/// Source generator type. Lund \c type == 1 means propagated in Geant4.
	int type = 1;

	/// Number of copies represented by this record.
	int multiplicity = 1;

	/// Momentum magnitude in GEMC internal units.
	double p = 0;

	/// Polar angle in GEMC internal angular units.
	double theta = 0;

	/// Azimuthal angle in GEMC internal angular units.
	double phi = 0;

	/// Vertex x coordinate in GEMC internal length units.
	double vx = 0;

	/// Vertex y coordinate in GEMC internal length units.
	double vy = 0;

	/// Vertex z coordinate in GEMC internal length units.
	double vz = 0;
};

/**
 * \brief Event-local list of generated-particle records for output.
 */
using GParticleRecordEvent  = std::vector<GParticleRecord>;

/**
 * \brief Sequence of generated-particle record events indexed by event number.
 */
using GParticleRecordEvents = std::vector<GParticleRecordEvent>;

/**
 * \brief One configured \c -gparticlefile source.
 *
 * The \c format token selects either a built-in reader, such as \c lund, or a
 * dynamic plugin named by \ref gparticlePluginName(). The \c filename value is
 * passed unchanged to the reader.
 */
struct GParticleSourceDefinition
{
	/// \brief Creates an empty source definition.
	GParticleSourceDefinition() = default;

	/**
	 * \brief Creates a source definition from a format token and filename.
	 *
	 * \param f Source format token.
	 * \param n Source filename.
	 */
	GParticleSourceDefinition(std::string f, std::string n)
		: format(std::move(f)), filename(std::move(n)) {
	}

	/// Source format token, for example \c lund.
	std::string format;

	/// Source filename.
	std::string filename;

	/**
	 * \brief Returns the dynamic plugin name associated with this format.
	 *
	 * \return Plugin name following the \c gparticle_<format>_plugin convention.
	 */
	[[nodiscard]] std::string gparticlePluginName() const { return "gparticle_" + format + "_plugin"; }
};

/**
 * \brief Abstract base class for gparticle file readers.
 *
 * Reader plugins translate file-backed generator input into two related views:
 * - \ref GParticleEvents for particles that are propagated to Geant4.
 * - \ref GParticleRecordEvents for generated-particle output banks, including
 *   particles that cannot or should not be propagated.
 *
 * Built-in and dynamic readers share the same factory contract through
 * \ref GBase and \ref instantiate().
 */
class GParticleReader : public GBase<GParticleReader>
{
public:
	/**
	 * \brief Constructs a reader with the shared options container.
	 *
	 * \param gopts Parsed options used by base logging and derived readers.
	 */
	explicit GParticleReader(const std::shared_ptr<GOptions>& gopts);

	/// \brief Virtual destructor for plugin use through base pointers.
	~GParticleReader() override = default;

	/**
	 * \brief Loads all propagated particles as one flattened list.
	 *
	 * This legacy view is equivalent to flattening \ref loadParticleEvents()
	 * with the default propagated-only behavior.
	 *
	 * \param source Source definition to read.
	 * \param logger Logger used for diagnostics.
	 * \return Vector of Geant4-propagated particles.
	 */
	virtual std::vector<GparticlePtr> loadParticles(const GParticleSourceDefinition& source,
	                                                const std::shared_ptr<GLogger>& logger) = 0;

	/**
	 * \brief Loads file-backed events as \ref Gparticle objects.
	 *
	 * Readers should return only rows that can be represented as \ref Gparticle
	 * objects and shot by Geant4 when \p propagated_only is \c true. Formats that
	 * carry a source type field may include additional rows when
	 * \p propagated_only is \c false, provided those rows are still valid
	 * Geant4 particles.
	 *
	 * \param source Source definition to read.
	 * \param logger Logger used for diagnostics.
	 * \param propagated_only Whether to restrict the result to particles propagated in Geant4.
	 * \return File-backed generated events as Geant4-shootable particles.
	 */
	virtual GParticleEvents loadParticleEvents(const GParticleSourceDefinition& source,
	                                           const std::shared_ptr<GLogger>& logger,
	                                           bool propagated_only = true);

	/**
	 * \brief Loads file-backed events as output-bank records.
	 *
	 * This view preserves all generator rows that can be parsed from the source
	 * file, including rows that are not propagated in Geant4 and rows whose ids
	 * are not known to \c G4ParticleTable.
	 *
	 * \param source Source definition to read.
	 * \param logger Logger used for diagnostics.
	 * \return File-backed generated events as serialization records.
	 */
	virtual GParticleRecordEvents loadParticleRecordEvents(const GParticleSourceDefinition& source,
	                                                       const std::shared_ptr<GLogger>& logger);

	/// \brief Reader plugins currently share the base logger setup.
	void set_loggers([[maybe_unused]] const std::shared_ptr<GOptions>& gopts) {
	}

	/**
	 * \brief Instantiates a dynamic reader plugin from a library handle.
	 *
	 * The plugin must export \c GParticleReaderFactory.
	 *
	 * \param h Dynamic library handle.
	 * \param gopts Parsed options passed to the plugin factory.
	 * \return Newly allocated reader instance owned by the caller.
	 */
	static GParticleReader* instantiate(dlhandle h, std::shared_ptr<GOptions> gopts);
};

namespace gparticle {
/**
 * \brief Parses all configured \c -gparticlefile entries.
 *
 * \param gopts Parsed options containing the optional \c gparticlefile node.
 * \return Source definitions in configuration order.
 */
std::vector<GParticleSourceDefinition> getGParticleSourceDefinitions(const std::shared_ptr<GOptions>& gopts);

/**
 * \brief Loads propagated particles from all configured file sources.
 *
 * \param gopts Parsed options.
 * \param logger Logger used for diagnostics.
 * \return Flattened vector of Geant4-propagated particles.
 */
std::vector<GparticlePtr> getGParticlesFromSources(const std::shared_ptr<GOptions>& gopts,
                                                   std::shared_ptr<GLogger>&        logger);

/**
 * \brief Loads event-indexed propagated particles from all configured file sources.
 *
 * \param gopts Parsed options.
 * \param logger Logger used for diagnostics.
 * \param propagated_only Whether to restrict file rows to Geant4-propagated particles.
 * \return Event-indexed generated particles.
 */
GParticleEvents getGParticleEventsFromSources(const std::shared_ptr<GOptions>& gopts,
                                              std::shared_ptr<GLogger>&        logger,
                                              bool propagated_only = true);

/**
 * \brief Loads event-indexed generated-particle records from all configured file sources.
 *
 * This is the data source for the \c generated output bank. It preserves every
 * parsed file particle, including particles that are not propagated in Geant4.
 *
 * \param gopts Parsed options.
 * \param logger Logger used for diagnostics.
 * \return Event-indexed generated-particle records.
 */
GParticleRecordEvents getGParticleRecordEventsFromSources(const std::shared_ptr<GOptions>& gopts,
                                                          std::shared_ptr<GLogger>&        logger);

/**
 * \brief Returns built-in file-reader format tokens.
 *
 * \return Static list of built-in reader formats.
 */
const std::vector<std::string>& supported_static_reader_formats();
}
