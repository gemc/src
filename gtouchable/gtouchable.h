#pragma once

// gemc
#include "gbase.h"

// c++
#include <vector>
#include <string>
#include <memory>

// gtouchable
#include "gtouchable_options.h"
#include "gtouchableConventions.h"

// - readout: electronic Time Window is the discriminating factor.
//   parameters and hitBitSet determined by defineReadoutSpecs in the plugin
// - flux: track id is the discriminating factor, standard true infos variable
// - particleCounter: no other discriminating factors, standard true infos variable
// - dosimeter: track id is the discriminating factor, radiation digitization


/**
 * @brief Enumeration representing the type of a touchable sensitive element.
 *
 * The type determines the **secondary** discriminating rule used after the identity vector:
 * - \c readout uses the electronics time-cell index.
 * - \c flux and \c dosimeter use the track id.
 * - \c particleCounter requires no additional discriminator beyond the identity vector.
 *
 * The mapping from a digitization string to a \c GTouchableType is implemented in the \c GTouchable constructors,
 * using the constants \c FLUXNAME, \c COUNTERNAME, and \c DOSIMETERNAME.
 */
enum GTouchableType
{
	readout,         ///< Electronic readout with time-window discrimination (time-cell index).
	flux,            ///< Flux-like discrimination using track id.
	particleCounter, ///< Identity vector only; no additional discriminating factor.
	dosimeter        ///< Radiation digitization; discrimination using track id.
};

// ------------------------------------------------------------------------
// Convert enum to string for logging / debugging.
// ------------------------------------------------------------------------
namespace gtouchable {
/**
 * @brief Converts a \c GTouchableType value to a stable string for logging.
 *
 * The returned strings match the digitization type constants where applicable:
 * - \c flux -> \c FLUXNAME
 * - \c particleCounter -> \c COUNTERNAME
 * - \c dosimeter -> \c DOSIMETERNAME
 *
 * @param t The touchable type.
 * @return A C-string suitable for log messages.
 */
inline const char* to_string(GTouchableType t) {
	switch (t) {
	case GTouchableType::readout: return "readout";
	case GTouchableType::flux: return FLUXNAME;
	case GTouchableType::particleCounter: return COUNTERNAME;
	case GTouchableType::dosimeter: return DOSIMETERNAME;
	default: return "unknown-gtouchable";
	}
}
}

/**
 * @brief A single (name,value) identifier element used to build a touchable identity vector.
 *
 * A \c GTouchable identity is an ordered vector of these identifiers, typically created by parsing a user-facing
 * identity string, e.g. \c "sector: 2, layer: 4, wire: 33".
 *
 * Important:
 * - Equality compares only the numeric value, because identity structures are assumed to match positionally
 *   (same detector sensitivity implies the same identifier schema/order).
 */
struct GIdentifier
{
public:
	/**
	 * @brief Constructs a \c GIdentifier.
	 *
	 * @param n Identifier name (e.g. \c "sector").
	 * @param v Identifier value (e.g. \c 2).
	 */
	GIdentifier(const std::string& n, int v) : idName{n}, idValue{v} {
	}

	/**
	 * @brief Compares identifiers by value only.
	 *
	 * This is used during \c GTouchable comparisons, where the identifier schema is expected to match.
	 *
	 * @param gid The identifier to compare with.
	 * @return True if the numeric values match.
	 */
	bool operator==(const GIdentifier& gid) const { return this->idValue == gid.idValue; }

	/**
	 * @brief Returns the identifier name.
	 * @return The identifier name.
	 */
	[[nodiscard]] inline std::string getName() const { return idName; }

	/**
	 * @brief Returns the identifier value.
	 * @return The identifier value.
	 */
	[[nodiscard]] inline int getValue() const { return idValue; }

private:
	std::string idName;  ///< Identifier name (human-readable label).
	int         idValue; ///< Identifier value (numeric discriminator).

	/// Stream output helper used in logs and diagnostics.
	friend std::ostream& operator<<(std::ostream& stream, const GIdentifier& gidentifier);
};


/**
 * @brief Represents a touchable sensitive detector element used as a hit-collection discriminator.
 *
 * A \c GTouchable acts as a compact “address” for sensitive detector elements during hit processing and digitization.
 * It is commonly used as a key when deciding whether a newly produced hit should:
 * - merge into an existing hit collection entry, or
 * - create a new entry.
 *
 * \section gtouchable_comparison Comparison semantics
 * The equality operator implements a two-stage comparison:
 * 1. Compare the identity vector values positionally (same size and same identifier values).
 * 2. If identities match, apply a type-specific discriminator:
 *    - \c readout compares \c stepTimeAtElectronicsIndex
 *    - \c flux compares \c trackId
 *    - \c dosimeter compares \c trackId
 *    - \c particleCounter always matches once identities match
 *
 * The identity vector size mismatch is considered an exceptional situation and is logged at debug level.
 */
class GTouchable : public GBase<GTouchable>
{
public:
	GTouchable(const GTouchable&)            = default;
	GTouchable& operator=(const GTouchable&) = default;

	/**
	 * @brief Constructs a \c GTouchable using module options.
	 *
	 * This constructor is used when a module options object is available and a module-scoped logger should be
	 * created/used by the base class.
	 *
	 * Called from detector construction code when building the sensitive detector registry.
	 *
	 * @param gopt Options container used to configure logging and module behavior.
	 * @param digitization Digitization type string (e.g. \c "readout", \c FLUXNAME, \c COUNTERNAME).
	 * @param gidentityString Identity specification string, e.g. \c "sector: 2, layer: 4, wire: 33".
	 * @param dimensions Physical dimensions of the detector element (module-defined convention).
	 */
	GTouchable(const std::shared_ptr<GOptions>& gopt,
	           const std::string&               digitization,
	           const std::string&               gidentityString,
	           const std::vector<double>&       dimensions);

	/**
	 * @brief Constructs a \c GTouchable using an existing logger.
	 *
	 * This constructor is useful when a caller already owns a configured logger instance (for example in tests
	 * or in code that wants to share a logger across multiple objects).
	 *
	 * @param logger Logger instance used for diagnostics.
	 * @param digitization Digitization type string (e.g. \c "readout", \c FLUXNAME, \c COUNTERNAME).
	 * @param gidentityString Identity specification string, e.g. \c "sector: 2, layer: 4, wire: 33".
	 * @param dimensions Physical dimensions of the detector element (module-defined convention).
	 */
	GTouchable(const std::shared_ptr<GLogger>& logger,
	           const std::string&              digitization,
	           const std::string&              gidentityString,
	           const std::vector<double>&      dimensions);

	/**
	 * @brief Copy constructor that preserves identity but updates the electronics time-cell index.
	 *
	 * This is used to create a new hit key when the identity matches but the time-cell differs, i.e. when a hit must
	 * be split by electronics time window.
	 *
	 * The copy uses the base class copy constructor and then copies the data members from \p base, replacing only
	 * \c stepTimeAtElectronicsIndex with \p newTimeIndex.
	 *
	 * @param base Existing touchable to copy from.
	 * @param newTimeIndex The updated electronics time-cell index.
	 */
	GTouchable(const std::shared_ptr<GTouchable>& base, int newTimeIndex)
		: GBase<GTouchable>(*base),
		  gType(base->gType),
		  gidentity(base->gidentity),
		  trackId(base->trackId),
		  eMultiplier(base->eMultiplier),
		  stepTimeAtElectronicsIndex(newTimeIndex) {
		log->debug(CONSTRUCTOR, "Copy-with-time-index", gtouchable::to_string(gType), " ", getIdentityString());
	}

	/**
	 * @brief Destructor with debug trace.
	 *
	 * The destructor logs the type and \ref GTouchable::getIdentityString "getIdentityString()"
	 * at debug level, which can be useful when diagnosing object lifetimes.
	 */
	~GTouchable() { log->debug(DESTRUCTOR, gtouchable::to_string(gType), " ", getIdentityString()); }

	/**
	 * @brief Compares two \c GTouchable instances using the module comparison semantics.
	 *
	 * @param gtouchable The touchable to compare with.
	 * @return True if the objects are considered equal (same identity and same type-specific discriminator).
	 */
	bool operator==(const GTouchable& gtouchable) const;

	/**
	 * @brief Assigns the track id used by \c flux and \c dosimeter discrimination.
	 *
	 * This value is typically set during hit processing when the simulation step is known.
	 *
	 * @param tid Track id to store in the touchable.
	 */
	inline void assignTrackId(int tid) { trackId = tid; }

	/**
	 * @brief Returns the energy multiplier used for energy sharing.
	 *
	 * The multiplier is typically set by digitization logic (for example when distributing energy across
	 * multiple readout cells). The default is 1.
	 *
	 * @return The energy multiplier.
	 */
	[[nodiscard]] inline double getEnergyMultiplier() const { return eMultiplier; }

	/**
	 * @brief Assigns the electronics time-cell index used by \c readout discrimination.
	 *
	 * @param timeIndex Electronics time-cell index that represents the readout time window.
	 */
	inline void assignStepTimeAtElectronicsIndex(int timeIndex) { stepTimeAtElectronicsIndex = timeIndex; }

	/**
	 * @brief Returns the electronics time-cell index.
	 *
	 * @return The stored time-cell index, or \c GTOUCHABLEUNSETTIMEINDEX if not assigned yet.
	 */
	[[nodiscard]] inline int getStepTimeAtElectronicsIndex() const { return stepTimeAtElectronicsIndex; }

	/**
	 * @brief Returns a copy of the identity vector.
	 *
	 * @return The identity vector as a \c std::vector of \c GIdentifier.
	 */
	[[nodiscard]] inline std::vector<GIdentifier> getIdentity() const { return gidentity; }

	/**
	 * @brief Builds a human-readable identity string from the stored identifiers.
	 *
	 * The returned string is constructed by concatenating each identifier as:
	 * \c "<name>: <value> " (note the trailing space).
	 *
	 * @return A human-readable identity string.
	 */
	[[nodiscard]] inline std::string getIdentityString() const {
		std::string idString;
		for (const auto& id : gidentity) { idString += id.getName() + ": " + std::to_string(id.getValue()) + " "; }
		return idString;
	}

	/**
	 * @brief Returns the detector dimensions stored at construction time.
	 *
	 * Dimensions are stored verbatim and interpreted by module-specific digitization logic.
	 *
	 * @return A vector containing the dimensions.
	 */
	[[nodiscard]] inline std::vector<double> getDetectorDimensions() const { return detectorDimensions; }

	/**
	 * @brief Checks whether this touchable exists in a vector using \c operator== semantics.
	 *
	 * This is a convenience helper mainly used for diagnostics and validation logic.
	 * It logs level-2 informational messages indicating whether the touchable was found.
	 *
	 * @param v Vector of touchables to scan.
	 * @return True if a matching touchable is found, false otherwise.
	 */
	[[nodiscard]] bool exists_in_vector(const std::vector<GTouchable>& v) const {
		for (const auto& gt : v) {
			if (*this == gt) {
				log->info(2, "GTouchable", this, " exists in vector.");
				return true;
			}
		}
		log->info(2, "GTouchable", this, " does not exist in vector.");

		return false;
	}

	/**
	 * @brief Creates a synthetic \c readout touchable for testing (options-based).
	 *
	 * The generated identity uses a deterministic pattern based on a process-wide atomic counter:
	 * - sector cycles in [1..6]
	 * - paddle cycles in [1..20]
	 *
	 * @param gopt Options container used to configure logging and module behavior.
	 * @return A newly created test touchable.
	 */
	static std::shared_ptr<GTouchable> create(const std::shared_ptr<GOptions>& gopt) {
		int         touchableNumber = globalGTouchableCounter.fetch_add(1, std::memory_order_relaxed);
		int         sector          = (touchableNumber % 6) + 1;
		int         paddle          = (touchableNumber % 20) + 1;
		std::string identity        = "sector: " + std::to_string(sector) + ", paddle: " + std::to_string(paddle);
		const auto& dimensions      = {10.0, 20.0, 30.0};

		return std::make_shared<GTouchable>(gopt, "readout", identity, dimensions);
	}

	/**
	 * @brief Creates a synthetic \c readout touchable for testing (logger-based).
	 *
	 * The generated identity uses a deterministic pattern based on a process-wide atomic counter:
	 * - sector cycles in [1..6]
	 * - paddle cycles in [1..20]
	 *
	 * @param logger Logger instance used for diagnostics.
	 * @return A newly created test touchable.
	 */
	static std::shared_ptr<GTouchable> create(const std::shared_ptr<GLogger>& logger) {
		int         touchableNumber = globalGTouchableCounter.fetch_add(1, std::memory_order_relaxed);
		int         sector          = (touchableNumber % 6) + 1;
		int         paddle          = (touchableNumber % 20) + 1;
		std::string identity        = "sector: " + std::to_string(sector) + ", paddle: " + std::to_string(paddle);
		const auto& dimensions      = {10.0, 20.0, 30.0};

		return std::make_shared<GTouchable>(logger, "readout", identity, dimensions);
	}

private:
	GTouchableType gType; ///< Touchable type controlling the secondary discriminator.
	std::vector<GIdentifier> gidentity; ///< Ordered identity vector defining the detector element address.
	int trackId; ///< Track id used for \c flux and \c dosimeter discrimination.
	double eMultiplier; ///< Energy multiplier for energy sharing (default 1; assigned by digitization).
	int stepTimeAtElectronicsIndex; ///< Readout time-cell index used for \c readout discrimination.
	std::vector<double> detectorDimensions; ///< Detector dimensions stored for digitization use.

	/// Stream output helper used in logs and diagnostics.
	friend std::ostream& operator<<(std::ostream& stream, const GTouchable& gtouchable);

	/// Static thread-safe counter used by \ref GTouchable::create "create()" to generate deterministic test identities.
	static std::atomic<int> globalGTouchableCounter;
};
