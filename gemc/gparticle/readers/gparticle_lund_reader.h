#pragma once

// gparticle
#include "../gparticle_reader.h"

/**
 * \brief Built-in reader for Lund generated-particle files.
 *
 * The Lund reader produces both Geant4-shootable particles and output-bank
 * records. The propagated \ref GParticleEvents view keeps only rows with
 * Lund \c type == 1 by default. The \ref GParticleRecordEvents view preserves
 * every parsed particle row, including non-propagated rows and ids not known
 * to Geant4, so the event output can populate the \c generated bank.
 */
class GParticleLundReader : public GParticleReader
{
public:
	/// \brief Inherit the base reader constructor.
	using GParticleReader::GParticleReader;

	/**
	 * \brief Loads all propagated Lund particles as one flattened list.
	 *
	 * \param source Lund file source definition.
	 * \param logger Logger used for diagnostics.
	 * \return Flattened vector of propagated \ref Gparticle objects.
	 */
	std::vector<GparticlePtr> loadParticles(const GParticleSourceDefinition& source,
	                                        const std::shared_ptr<GLogger>& logger) override;

	/**
	 * \brief Loads Lund events as Geant4-shootable particles.
	 *
	 * \param source Lund file source definition.
	 * \param logger Logger used for diagnostics.
	 * \param propagated_only When \c true, include only Lund rows with \c type == 1.
	 * \return Event-indexed propagated particles.
	 */
	GParticleEvents loadParticleEvents(const GParticleSourceDefinition& source,
	                                   const std::shared_ptr<GLogger>& logger,
	                                   bool propagated_only = true) override;

	/**
	 * \brief Loads Lund events as generated-particle records.
	 *
	 * This method preserves all parsed particle rows. Unknown particle ids are
	 * represented by their numeric id in the record name field.
	 *
	 * \param source Lund file source definition.
	 * \param logger Logger used for diagnostics.
	 * \return Event-indexed generated-particle records.
	 */
	GParticleRecordEvents loadParticleRecordEvents(const GParticleSourceDefinition& source,
	                                               const std::shared_ptr<GLogger>& logger) override;
};
