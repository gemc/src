#pragma once

/**
 * \file gEventDataCollection.h
 * \brief Defines GEventDataCollection, the event-level aggregation of detector hit data.
 *
 * \details
 * GEventDataCollection groups detector-local hit data, generated-particle metadata, and optional
 * track provenance produced during one event.
 *
 * Primary organization:
 * \code
 * detector name (std::string) -> GDataCollection
 *                                - vector<unique_ptr<GTrueInfoData>>
 *                                - vector<unique_ptr<GDigitizedData>>
 *
 * generated particle banks:
 *                                - generated
 *                                - generated_tracked
 * optional track provenance bank:
 *                                - ancestors
 * \endcode
 *
 * Event semantics:
 * - each insertion appends one hit-side object to the target detector entry
 * - detector entries are created lazily on first use
 * - ownership of inserted objects is transferred to this collection
 *
 * Typical workflow:
 * - create one event container
 * - add truth and digitized objects as hits are produced
 * - pass the completed event container downstream for output, analysis, or run integration
 */

#include <gemc/gdata/gDataCollection.h>
#include "gEventHeader.h"

// C++
#include <map>
#include <string>
#include <vector>

constexpr const char* GEVENTDATA_LOGGER = "gevent_data";

/**
 * \brief Serialization record for one generated particle in event output.
 *
 * This lightweight structure mirrors \c GParticleRecord without depending on
 * the gparticle module from gdata. It is used by event streamers to publish
 * generated-particle metadata alongside detector hit banks.
 *
 * The \c generated bank contains runtime records for particles propagated in
 * Geant4 plus source-only file rows that are not propagated. The
 * \c generated_tracked bank contains only runtime records for particles
 * propagated in Geant4, normally rows with \c type == 1 for Lund input.
 */
struct GGeneratedParticleData
{
	/// Particle name when known, otherwise a source-format identifier.
	std::string name;

	/// PDG id or source particle id.
	int pid = 0;

	/// Generator source type. For Lund input, \c type == 1 means Geant4-propagated.
	int type = 1;

	/// Number of copies represented by this row.
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
 * \brief Event-local generated-particle bank.
 */
using GGeneratedParticleBank = std::vector<GGeneratedParticleData>;

/**
 * \brief Initial state of one hit-producing track or one of its ancestors.
 */
struct GAncestorData
{
	int    pid = 0;
	int    tid = 0;
	int    mtid = 0;
	double trackE = 0;
	double px = 0;
	double py = 0;
	double pz = 0;
	double vx = 0;
	double vy = 0;
	double vz = 0;
};

using GAncestorBank = std::vector<GAncestorData>;

namespace gevent_data {

/**
 * \brief Aggregates the option groups needed by event-level data containers.
 *
 * \details
 * The returned bundle includes:
 * - event-header options
 * - true-data options
 * - digitized-data options
 * - touchable-related options used by example identity creation
 *
 * \return Composite options group rooted at \c GEVENTDATA_LOGGER.
 */
inline auto defineOptions() -> GOptions {
	auto goptions = GOptions(GEVENTDATA_LOGGER);
	goptions      += geventheader::defineOptions();
	goptions      += gtrue_data::defineOptions();
	goptions      += gdigi_data::defineOptions();
	goptions      += gtouchable::defineOptions();
	return goptions;
}

} // namespace gevent_data

/**
 * \defgroup gdata_event_collection GData event collection
 * \brief Event-level ownership of detector-local truth and digitized hit data.
 *
 * \details
 * This topic documents the event container that owns one event header together with the map of
 * detector-local GDataCollection objects plus optional generated-particle and ancestor banks.
 * It is the main aggregation layer used before run-level integration.
 */

/**
 * \brief Owns all detector-local data for one event.
 * \ingroup gdata_event_collection
 *
 * \details
 * The object combines:
 * - one owned GEventHeader describing the event
 * - one map of detector names to GDataCollection instances
 * - optional generated-particle banks used by event streamers
 * - an optional ancestor bank used when track provenance output is requested
 *
 * Each detector entry can contain:
 * - zero or more truth objects
 * - zero or more digitized objects
 *
 * The class does not enforce structural invariants such as matching truth and digitized counts.
 * Applications that require such guarantees should validate them at a higher level.
 */
class GEventDataCollection : public GBase<GEventDataCollection>
{
public:
	/**
	 * \brief Constructs an event data collection with an owned header.
	 *
	 * \details
	 * Ownership of \p header is transferred to the collection.
	 *
	 * \param gopts  Shared options used to configure logging and related behavior.
	 * \param header Owned event header.
	 */
	GEventDataCollection(const std::shared_ptr<GOptions>& gopts, std::unique_ptr<GEventHeader> header)
		: GBase(gopts, GEVENTDATA_LOGGER), gevent_header(std::move(header)) {
	}

	/**
	 * \brief Appends one truth object to the specified detector entry.
	 *
	 * \details
	 * If the detector key does not exist yet, a new GDataCollection is created automatically.
	 * Ownership of \p data is transferred to the target detector entry.
	 *
	 * \param sdName Sensitive detector name used as the map key.
	 * \param data   Truth object to store.
	 */
	void addDetectorTrueInfoData(const std::string& sdName, std::unique_ptr<GTrueInfoData> data);

	/**
	 * \brief Appends one digitized object to the specified detector entry.
	 *
	 * \details
	 * If the detector key does not exist yet, a new GDataCollection is created automatically.
	 * Ownership of \p data is transferred to the target detector entry.
	 *
	 * \param sdName Sensitive detector name used as the map key.
	 * \param data   Digitized object to store.
	 */
	void addDetectorDigitizedData(const std::string& sdName, std::unique_ptr<GDigitizedData> data);

	/**
	 * \brief Returns read-only access to the owned event header.
	 *
	 * \return Const reference to the owned event-header pointer.
	 */
	[[nodiscard]] auto getHeader() const -> const std::unique_ptr<GEventHeader>& { return gevent_header; }

	/**
	 * \brief Returns read-only access to the detector map for this event.
	 *
	 * \details
	 * Keys are sensitive detector names and values are per-detector GDataCollection instances.
	 *
	 * \return Const reference to the detector map.
	 */
	[[nodiscard]] auto getDataCollectionMap() const
	    -> const std::map<std::string, std::unique_ptr<GDataCollection>>& {
		return gdataCollectionMap;
	}

	/**
	 * \brief Stores the full generated-particle bank for this event.
	 *
	 * The bank named \c generated contains runtime records for particles
	 * propagated in Geant4 plus source-only file rows that are not propagated.
	 *
	 * \param particles Generated-particle rows to store.
	 */
	void setGeneratedParticles(GGeneratedParticleBank particles) {
		generated_particles = std::move(particles);
	}

	/**
	 * \brief Stores the Geant4-tracked generated-particle bank for this event.
	 *
	 * The bank named \c generated_tracked contains only runtime records for
	 * particles propagated in Geant4, normally those with source \c type == 1.
	 *
	 * \param particles Generated-particle rows to store.
	 */
	void setGeneratedTrackedParticles(GGeneratedParticleBank particles) {
		generated_tracked_particles = std::move(particles);
	}

	/**
	 * \brief Returns the full generated-particle bank.
	 *
	 * \return Rows published as the \c generated output bank.
	 */
	[[nodiscard]] const GGeneratedParticleBank& getGeneratedParticles() const { return generated_particles; }

	/**
	 * \brief Returns the Geant4-tracked generated-particle bank.
	 *
	 * \return Rows published as the \c generated_tracked output bank.
	 */
	[[nodiscard]] const GGeneratedParticleBank& getGeneratedTrackedParticles() const {
		return generated_tracked_particles;
	}

	/** \brief Stores the initial states of hit-producing tracks and their ancestors. */
	void setAncestors(GAncestorBank ancestors) {
		ancestor_particles = std::move(ancestors);
		ancestor_bank_enabled = true;
	}

	/** \brief Returns the event-local ancestor bank. */
	[[nodiscard]] const GAncestorBank& getAncestors() const { return ancestor_particles; }

	/** \brief Reports whether ancestor output was requested for this event. */
	[[nodiscard]] bool hasAncestorBank() const { return ancestor_bank_enabled; }

	/**
	 * \brief Returns the event number stored in the owned header.
	 *
	 * \details
	 * This is a convenience wrapper around the header accessor.
	 *
	 * \return Event number.
	 */
	[[nodiscard]] auto getEventNumber() const -> int { return gevent_header->getG4LocalEvn(); }

	/**
	 * \brief Creates a minimal example event containing one detector entry and one hit pair.
	 *
	 * \details
	 * This helper is intended for examples and tests.
	 * It creates:
	 * - a fresh GEventHeader
	 * - a new event container
	 * - one digitized object under detector \c ctof
	 * - one truth object under detector \c ctof
	 *
	 * \param gopts Shared options.
	 * \return Shared pointer to the created event collection.
	 */
	static auto create(const std::shared_ptr<GOptions>& gopts) -> std::shared_ptr<GEventDataCollection> {
		auto header = GEventHeader::create(gopts);
		auto edc    = std::make_shared<GEventDataCollection>(gopts, std::move(header));

		auto digi_data = GDigitizedData::create(gopts);
		auto true_data = GTrueInfoData::create(gopts);

		edc->addDetectorDigitizedData("ctof", std::move(digi_data));
		edc->addDetectorTrueInfoData("ctof", std::move(true_data));

		return edc;
	}

private:
	/// Owned event header describing this event.
	std::unique_ptr<GEventHeader> gevent_header;

	/// Per-detector data map keyed by sensitive detector name.
	std::map<std::string, std::unique_ptr<GDataCollection>> gdataCollectionMap;

	/// Full generated-particle bank published as \c generated.
	GGeneratedParticleBank generated_particles;

	/// Geant4-propagated generated-particle bank published as \c generated_tracked.
	GGeneratedParticleBank generated_tracked_particles;

	/// Initial states of hit-producing tracks and their ancestors.
	GAncestorBank ancestor_particles;
	bool          ancestor_bank_enabled = false;

	/// Static thread-safe counter reserved for tests or future example helpers.
	static std::atomic<int> globalEventDataCollectionCounter;
};
