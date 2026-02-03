#pragma once

// HitBitSet definition
#include "ghitConventions.h"

// geant4
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"

#include "G4ThreeVector.hh"
#include "G4Step.hh"
#include "G4Colour.hh"

// gemc
#include "gtouchable.h"

// c++
#include <optional>
#include <atomic>

/**
 * \class GHit
 * \brief Stores step-by-step and aggregated information for a detector hit.
 *
 * A \c GHit is a \c G4VHit that accumulates per-step quantities while a track
 * traverses a sensitive detector element and deposits energy.
 *
 * Conceptually, this class has two layers of information:
 * - **Per-step vectors**: always-collected quantities (energy deposition, time, local/global positions)
 *   plus optional quantities controlled by \c HitBitSet.
 * - **Aggregated quantities**: totals/averages (e.g., total energy deposited, average time,
 *   average positions, representative process name) computed lazily from the per-step vectors.
 *
 * The optional information is controlled by \c HitBitSet (see ghitConventions.h : bit meanings
 * and expected future extensions).
 *
 * \note This class does not own the sensitive-element description. The associated \c GTouchable
 * is stored as a \c std::shared_ptr so that the hit can be compared against other hits and can
 * query identity/dimensions.
 */
class GHit : public G4VHit
{
public:
	/**
	 * \brief Construct a hit container and optionally seed it from a step.
	 *
	 * This constructor initializes the hit bookkeeping and, if \p thisStep is not null,
	 * immediately records per-step information for that step (both always-present data and any
	 * enabled optional data in \p hbs).
	 *
	 * \param gt Pointer to the \c GTouchable describing the sensitive element producing the hit.
	 * \param hbs Bitset selecting which optional hit information is recorded in addition to the always-present fields.
	 * \param thisStep Optional \c G4Step used to seed the hit with an initial step record (default: null).
	 * \param cScheme Visualization color scheme name (default: "default"). The current implementation uses
	 *                a simple hard-coded scheme but keeps this field for future expansion.
	 */
	GHit(std::shared_ptr<GTouchable> gt, HitBitSet hbs, const G4Step* thisStep = nullptr,
	     const std::string&          cScheme                                   = "default");

	/**
	 * \brief Destructor.
	 */
	~GHit() override = default;

	/**
	 * \brief Allocate a \c GHit via the \c G4Allocator associated with this type.
	 * \return Pointer to the allocated storage.
	 *
	 * \note The allocator is thread-local (see \c GHitAllocator below).
	 */
	inline void* operator new(size_t);

	/**
	 * \brief Deallocate a \c GHit via the \c G4Allocator associated with this type.
	 */
	inline void operator delete(void*);

	/**
	 * \brief Visualize the hit using \c Geant4 visualization primitives.
	 *
	 * This draws a circle at the first recorded global position and selects visual attributes
	 * based on the total energy deposited.
	 *
	 * \note If no visualization manager is available, or if the hit has no recorded positions,
	 *       the method returns without performing any drawing.
	 */
	void Draw() override;

	/**
	 * \brief Compare this hit against another hit by sensitive-element identity.
	 *
	 * Two hits are considered the "same" if their associated \c GTouchable objects compare equal
	 * (i.e. they refer to the same detector element identity according to \c GTouchable equality).
	 *
	 * \param hit Pointer to the candidate hit to compare against (may be null).
	 * \return True if \p hit is not null and the associated \c GTouchable matches.
	 */
	[[nodiscard]] bool is_same_hit(const GHit* hit) const;

private:
	/**
	 * \brief Visualization colors used by \ref Draw().
	 *
	 * - \c colour_hit is used for hits with non-zero total energy deposition.
	 * - \c colour_passby is used for trajectories crossing without depositing energy.
	 * - \c colour_touch is currently reserved for future use (e.g. touchable outline).
	 *
	 * \note These are configured by \ref setColorSchema().
	 */
	G4Colour colour_touch, colour_hit, colour_passby;

	/**
	 * \brief Configure visualization colors for the current \ref colorSchema.
	 *
	 * \return The current implementation always returns false (kept for compatibility / future use).
	 */
	bool setColorSchema();

	/// Human-readable color scheme identifier (used by \ref setColorSchema()).
	std::string colorSchema;

	/**
	 * \brief Sensitive-element descriptor for this hit.
	 *
	 * Stored so that the hit can:
	 * - compare identity to other hits in \ref is_same_hit(),
	 * - report identity via \ref getGID() / \ref getTTID(),
	 * - report element dimensions via \ref getDetectorDimensions(),
	 * - apply detector-specific energy scaling via \c GTouchable::getEnergyMultiplier().
	 */
	std::shared_ptr<GTouchable> gtouchable;

	// -------------------------------------------------------------------------
	// Per-step data (vectors)
	// -------------------------------------------------------------------------

	/**
	 * \brief Energy deposited per step.
	 *
	 * Values are pushed in \ref addHitInfosForBitset() using:
	 * \c (step->GetTotalEnergyDeposit()) * (gtouchable->getEnergyMultiplier()).
	 */
	std::vector<double> edeps;

	/**
	 * \brief Global time per step.
	 *
	 * Values are derived from \c preStepPoint->GetGlobalTime().
	 */
	std::vector<double> times;

	/**
	 * \brief Global positions per step (world coordinates).
	 *
	 * Values are derived from \c preStepPoint->GetPosition().
	 */
	std::vector<G4ThreeVector> globalPositions;

	/**
	 * \brief Local positions per step (sensitive-element local coordinates).
	 *
	 * Computed by transforming the global position using the top transform of the touchable history.
	 */
	std::vector<G4ThreeVector> localPositions;

	// Optional per-step data, controlled by HitBitSet (see ghitConventions.h : meaning of each bit)

	/**
	 * \brief Particle PDG encodings per step (optional).
	 *
	 * Recorded when the corresponding \c HitBitSet bit is enabled.
	 */
	std::vector<int> pids;

	/**
	 * \brief Total energy per step (optional).
	 *
	 * Recorded when the corresponding \c HitBitSet bit is enabled.
	 */
	std::vector<double> Es;

	/**
	 * \brief Process name per step (optional).
	 *
	 * Recorded when the corresponding \c HitBitSet bit is enabled and a creator process exists.
	 * The aggregated representative process name is available via \ref getProcessName().
	 */
	std::vector<std::string> processNames;

	/**
	 * \brief Step length per step (optional, future extension).
	 *
	 * \note The bit is defined in the conventions, but the current implementation does not yet fill this vector.
	 */
	std::vector<double> stepSize;

	// -------------------------------------------------------------------------
	// Aggregated / calculated quantities (lazy)
	// -------------------------------------------------------------------------

	/**
	 * \brief Cached total energy deposited across all steps.
	 *
	 * This is computed the first time \ref getTotalEnergyDeposited() is called and then cached.
	 */
	std::optional<double> totalEnergyDeposited;

	/**
	 * \brief Cached average time across steps.
	 *
	 * The averaging is energy-weighted when the total deposited energy is non-zero; otherwise,
	 * it falls back to a simple average.
	 *
	 * \note This is initialized to an "uninitialized" sentinel value and computed on demand.
	 */
	double averageTime;

	/**
	 * \brief Cached energy-weighted (or arithmetic) average global position.
	 *
	 * \note This is initialized to an "uninitialized" sentinel vector and computed on demand.
	 */
	G4ThreeVector avgGlobalPosition;

	/**
	 * \brief Cached energy-weighted (or arithmetic) average local position.
	 *
	 * \note This is initialized to an "uninitialized" sentinel vector and computed on demand.
	 */
	G4ThreeVector avgLocalPosition;

	/**
	 * \brief Cached representative process name for the hit.
	 *
	 * The current implementation selects the first recorded process name (if any).
	 */
	std::string processName;

	/**
	 * \brief Add optional hit information for a specific bit.
	 *
	 * When \p test is true, this method extracts additional per-step information from \p thisStep
	 * corresponding to \p bitIndex and appends it to the relevant vectors.
	 *
	 * \param bitIndex Bit index within the \c HitBitSet.
	 * \param test True if the bit is enabled.
	 * \param thisStep Step providing the data.
	 * \return True if enabled and the method executed the bit handler; false otherwise.
	 */
	bool addHitInfosForBitIndex(size_t bitIndex, bool test, const G4Step* thisStep);

	/**
	 * \brief Thread-safe global counter used by \ref create() for test randomization only.
	 */
	static std::atomic<int> globalHitCounter;

public:
	// -------------------------------------------------------------------------
	// Inline accessors (returning copies by design)
	// -------------------------------------------------------------------------

	/**
	 * \brief Get per-step energy depositions.
	 * \return A copy of the vector of per-step deposited energies.
	 */
	[[nodiscard]] inline std::vector<double> getEdeps() const { return edeps; }

	/**
	 * \brief Get per-step global times.
	 * \return A copy of the vector of per-step times.
	 */
	[[nodiscard]] inline std::vector<double> getTimes() const { return times; }

	/**
	 * \brief Get per-step global positions.
	 * \return A copy of the vector of per-step global positions.
	 */
	[[nodiscard]] inline std::vector<G4ThreeVector> getGlobalPositions() const { return globalPositions; }

	/**
	 * \brief Get per-step local positions.
	 * \return A copy of the vector of per-step local positions.
	 */
	[[nodiscard]] inline std::vector<G4ThreeVector> getLocalPositions() const { return localPositions; }

	/**
	 * \brief Get per-step particle PDG encodings (when enabled).
	 * \return A copy of the vector of per-step particle IDs.
	 */
	[[nodiscard]] inline std::vector<int> getPids() const { return pids; }

	/**
	 * \brief Convenience accessor for the first particle ID.
	 * \return The first particle ID.
	 *
	 * \warning This assumes the internal \c pids vector is non-empty.
	 */
	[[nodiscard]] inline int getPid() const { return pids.front(); }

	/**
	 * \brief Get per-step total energies (when enabled).
	 * \return A copy of the vector of per-step energies.
	 */
	[[nodiscard]] inline std::vector<double> getEs() const { return Es; }

	/**
	 * \brief Convenience accessor for the first energy value.
	 * \return The first energy value.
	 *
	 * \warning This assumes the internal \c Es vector is non-empty.
	 */
	[[nodiscard]] inline double getE() const { return Es.front(); }

	/**
	 * \brief Number of recorded steps for the optional-energy vector.
	 * \return The size of the \c Es vector.
	 *
	 * \note Depending on the \c HitBitSet configuration, \c Es may remain empty even if
	 *       always-present vectors have entries.
	 */
	[[nodiscard]] inline size_t nsteps() const { return Es.size(); }

	/**
	 * \brief Get the representative process name for the hit.
	 * \return The cached representative process name string.
	 *
	 * \note This value is typically populated when \ref calculateInfosForBit() runs for bit 0,
	 *       or when average/total getters trigger calculations.
	 */
	[[nodiscard]] inline std::string getProcessName() const { return processName; }

	/**
	 * \brief Get the associated sensitive-element descriptor.
	 * \return A copy of the \c std::shared_ptr managing the \c GTouchable.
	 */
	[[nodiscard]] inline std::shared_ptr<GTouchable> getGTouchable() const { return gtouchable; }

	/**
	 * \brief Get the detector element identity.
	 * \return A vector of \c GIdentifier describing the sensitive-element identity.
	 *
	 * \note This forwards to \c GTouchable::getIdentity().
	 */
	[[nodiscard]] inline std::vector<GIdentifier> getGID() const { return gtouchable->getIdentity(); }

	/**
	 * \brief Get the sensitive-element dimensions.
	 * \return A vector of doubles representing the element dimensions.
	 *
	 * \note This forwards to \c GTouchable::getDetectorDimensions().
	 */
	[[nodiscard]] inline std::vector<double> getDetectorDimensions() const {
		return gtouchable->getDetectorDimensions();
	}

	// -------------------------------------------------------------------------
	// Aggregation / calculation API
	// -------------------------------------------------------------------------

	/**
	 * \brief Compute and cache derived information for the requested bit.
	 *
	 * This is primarily used to compute bit-0 derived quantities (total energy, average time,
	 * average local/global positions, representative process name).
	 *
	 * \param bit Bit index for which derived information should be computed.
	 *
	 * \note Bits beyond 0 are currently placeholders for future extensions (see ghitConventions.h : planned bits).
	 */
	void calculateInfosForBit(int bit);

	/**
	 * \brief Get the total deposited energy across all recorded steps.
	 * \return The summed energy deposition.
	 *
	 * This method caches the result the first time it is called.
	 */
	double getTotalEnergyDeposited();

	/**
	 * \brief Get the average time associated with the hit.
	 * \return The energy-weighted average of the time if total deposited energy is non-zero,
	 *         otherwise a simple arithmetic average.
	 *
	 * \note The internal cache uses an "uninitialized" sentinel; computation happens on first access.
	 */
	double getAverageTime();

	/**
	 * \brief Get the average local position of the hit.
	 * \return The averaged local position, energy-weighted if possible.
	 */
	G4ThreeVector getAvgLocalPosition();

	/**
	 * \brief Get the average global position of the hit.
	 * \return The averaged global position energy-weighted if possible.
	 *
	 * \note The function name is \c getAvgGlobaPosition() (missing 'l' in "Global") for historical reasons.
	 */
	G4ThreeVector getAvgGlobaPosition();

	// -------------------------------------------------------------------------
	// Hit filling / testing helpers
	// -------------------------------------------------------------------------

	/**
	 * \brief Append per-step information from a \c G4Step according to a bitset.
	 *
	 * Always records:
	 * - global and local positions,
	 * - energy deposited,
	 * - global time.
	 *
	 * Then iterates over each bit of \p hbs and conditionally records optional information
	 * via the internal bit handler.
	 *
	 * \param hbs Bitset selecting optional information to record.
	 * \param thisStep Step to extract per-step values from (must not be null).
	 */
	void addHitInfosForBitset(HitBitSet hbs, const G4Step* thisStep);

	/**
	 * \brief Randomize internal vectors for test-only usage.
	 *
	 * Fills vectors with pseudo-random positions, times, energies, and particle IDs.
	 *
	 * \param nsteps Number of pseudo-random steps to generate.
	 *
	 * \warning This is intended only for unit tests / examples. Do not use in production.
	 */
	void randomizeHitForTesting(int nsteps);

	/**
	 * \brief Get the touchable identity values as integers.
	 *
	 * Converts each \c GIdentifier returned by \ref getGID() into its integer value.
	 *
	 * \return Vector of integer identity values (one per identifier component).
	 */
	[[nodiscard]] std::vector<int> getTTID() const;

	/**
	 * \brief Create a fake hit for testing, using the current options.
	 *
	 * This uses \c GTouchable::create(gopts) to build a test touchable, constructs a hit with
	 * an empty \c HitBitSet, and then randomizes its contents using \ref randomizeHitForTesting().
	 *
	 * \param gopts Options object used to create the test \c GTouchable.
	 * \return Newly allocated \c GHit pointer. Ownership is transferred to the caller.
	 *
	 * \warning The returned pointer must be deleted by the caller.
	 */
	static GHit* create(const std::shared_ptr<GOptions>& gopts) {
		HitBitSet hitBitSet;
		auto      gt  = GTouchable::create(gopts);
		auto      hit = new GHit(gt, hitBitSet);
		// Randomize between 1 and 10 steps in a deterministic, thread-safe manner.
		hit->randomizeHitForTesting(1 + globalHitCounter.fetch_add(1, std::memory_order_relaxed) % 10);
		return hit;
	}
};

// MT definitions, as from:
// https://twiki.cern.ch/twiki/bin/view/Geant4/QuickMigrationGuideForGeant4V10
extern G4ThreadLocal G4Allocator<GHit>* GHitAllocator;
using GHitsCollection = G4THitsCollection<GHit>;

inline void* GHit::operator new(size_t) {
	if (!GHitAllocator) GHitAllocator = new G4Allocator<GHit>;
	return (void*)GHitAllocator->MallocSingle();
}

inline void GHit::operator delete(void* hit) {
	if (!GHitAllocator) { GHitAllocator = new G4Allocator<GHit>; }

	GHitAllocator->FreeSingle((GHit*)hit);
}
