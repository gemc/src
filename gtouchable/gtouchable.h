#pragma once

// gemc
#include "gbase.h"

// c++
#include <vector>
#include <string>
#include <memory>

// gtouchable
#include "gtouchable_options.h"

// - readout: electronic Time Window is the discriminating factor.
//   parameters and hitBitSet determined by defineReadoutSpecs in the plugin
// - flux: track id is the discriminating factor, standard true infos variable
// - particleCounter: no other discriminating factors, standard true infos variable
// - dosimeter: track id is the discriminating factor, radiation digitization


/**
 * @brief Enumeration representing the type of GTouchable element.
 *
 * Defines different types of sensitive detector elements.
 */
enum GTouchableType {
	readout,         ///< Electronic readout with time window discrimination.
	flux,            ///< Track-based discrimination.
	particleCounter, ///< No additional discriminating factors.
	dosimeter        ///< Radiation digitization using track id.
};

// ------------------------------------------------------------------------
// Convert enum to string for logging / debugging.
// ------------------------------------------------------------------------
namespace gtouchable {
inline const char* to_string(GTouchableType t) {
	switch (t) {
	case GTouchableType::readout: return "readout";
	case GTouchableType::flux: return "flux";
	case GTouchableType::particleCounter: return "particleCounter";
	case GTouchableType::dosimeter: return "dosimeter";
	default: return "unknown‑gtouchable";
	}
}
}

/**
 * @brief Represents a unique identifier for a sensitive detector element.
 *
 * Each identifier consists of a name and an integer value.
 */
struct GIdentifier {

public:
	/**
	 * @brief Constructs a GIdentifier.
	 * @param n The identifier name.
	 * @param v The identifier value.
	 */
	GIdentifier(const std::string& n, int v) : idName{n}, idValue{v} {
	}

	/**
	 * @brief Equality operator comparing only the identifier value.
	 * @param gid The GIdentifier to compare.
	 * @return True if the identifier values are equal.
	 */
	bool operator==(const GIdentifier& gid) const { return this->idValue == gid.idValue; }

	/**
	 * @brief Gets the identifier name.
	 * @return The identifier name.
	 */
	[[nodiscard]] inline std::string getName() const { return idName; }

	/**
	 * @brief Gets the identifier value.
	 * @return The identifier value.
	 */
	[[nodiscard]] inline int getValue() const { return idValue; }

private:
	std::string idName;
	int         idValue;

	/// Overloaded output operator for GIdentifier.
	friend std::ostream& operator<<(std::ostream& stream, const GIdentifier& gidentifier);

};


/**
 * @brief Represents a touchable sensitive detector element.
 *
 * The GTouchable class encapsulates properties and identification of a sensitive
 * detector element used in hit processing and digitization.
 * The key discriminating factors to check if the hit belongs to an existing hitcollection are:
 *  - the gidentity vector
 *  - the gType (if the gidentity vectors are the same)
 *  The algorithm is implemented in the operator== method.
 */
class GTouchable : public GBase<GTouchable> {

public:
	GTouchable(const GTouchable&)            = default;
	GTouchable& operator=(const GTouchable&) = default;

	/**
	* @brief Constructs a GTouchable from digitization and identifier strings.
	*
	* Called in GDetectorConstruction::ConstructSDandField to register a new
	* GTouchable in the sensitive detector map.
	*
	* @param gopt: GOptions
	* @param digitization The digitization type as a string.
	* @param gidentityString The string specifying the gidentity (e.g., "sector: 2, layer: 4, wire: 33").
	* @param dimensions The physical dimensions of the detector element.
	*/
	GTouchable(const std::shared_ptr<GOptions>& gopt,
	           const std::string&               digitization,
	           const std::string&               gidentityString,
	           const std::vector<double>&       dimensions);


	GTouchable(const std::shared_ptr<GLogger>& logger,
		   const std::string&               digitization,
		   const std::string&               gidentityString,
		   const std::vector<double>&       dimensions);
	/**
	* @brief Copy constructor for updating the electronic time index.
	* Used to create a new hit, in case the time indices differ.
	* Used in processTouchable when time indices differ.
	*
	* Used when the step time index of the hit is different from that of the GTouchable.
	* Base: shallow or deep depending on GBase’s copy
	*
	* @param base Pointer to the base GTouchable.
	* @param newTimeIndex The new electronics time index.
	*/
	GTouchable(const std::shared_ptr<GTouchable>& base, int newTimeIndex)
		: GBase<GTouchable>(*base),
		  gType(base->gType),
		  gidentity(base->gidentity),
		  trackId(base->trackId),
		  eMultiplier(base->eMultiplier),
		  stepTimeAtElectronicsIndex(newTimeIndex) { log->debug(CONSTRUCTOR, "Copy-with-time-index", gtouchable::to_string(gType), " ", getIdentityString()); }

	~GTouchable() { log->debug(DESTRUCTOR, gtouchable::to_string(gType), " ", getIdentityString()); }

	/**
	* @brief Equality operator comparing two GTouchable objects.
	*
	* Comparison is based on the gidentity vector and, if needed, on a type-specific property.
	*
	* @param gtouchable The GTouchable to compare with.
	* @return True if the objects are considered equal.
	*/
	bool operator==(const GTouchable& gtouchable) const;

	/**
	* @brief Assigns a track identifier.
	* @param tid The track id.
	* Called in GSensitiveDetector::ProcessHits
	*/
	inline void assignTrackId(int tid) { trackId = tid; }

	/**
	* @brief Gets the energy multiplier.
 	* @return The energy multiplier.
 	*/
	[[nodiscard]] inline double getEnergyMultiplier() const { return eMultiplier; }

	/**
 	* @brief Assigns the step time index used in electronics.
	* @param timeIndex The new time index.
 	*/
	inline void assignStepTimeAtElectronicsIndex(int timeIndex) { stepTimeAtElectronicsIndex = timeIndex; }

	/**
	* @brief Gets the electronics time index.
	* @return The step time index.
	*/
	[[nodiscard]] inline int getStepTimeAtElectronicsIndex() const { return stepTimeAtElectronicsIndex; }

	/**
	* @brief Returns the identifier vector.
	* @return A vector of GIdentifier objects.
	*/
	[[nodiscard]] inline std::vector<GIdentifier> getIdentity() const { return gidentity; }

	/**
	* @brief Returns a string formed by all identifiers.
	* @return A vector of GIdentifier objects.
	*/
	[[nodiscard]] inline std::string getIdentityString() const {
		std::string idString;
		for (const auto& id : gidentity) { idString += id.getName() + ": " + std::to_string(id.getValue()) + " "; }
		return idString;
	}

	/**
	* @brief Returns the detector dimensions.
	* @return A vector containing the dimensions.
	*/
	[[nodiscard]] inline std::vector<double> getDetectorDimensions() const { return detectorDimensions; }

	/**
	 * @brief Checks if the GTouchable is found in a vector of GTouchable objects.
	 * @param v The vector of GTouchable objects.
	 * @return True if the GTouchable is found in the vector.
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

	// create fake gtouchable for testing purposes, using sector and fake dimensions
	static std::shared_ptr<GTouchable> create(const std::shared_ptr<GOptions>& gopt) {
		int         touchableNumber = globalGTouchableCounter.fetch_add(1, std::memory_order_relaxed);
		int         sector          = (touchableNumber % 6) + 1;
		int         paddle          = (touchableNumber % 20) + 1;
		std::string identity        = "sector: " + std::to_string(sector) + ", paddle: " + std::to_string(paddle);
		const auto& dimensions      = {10.0, 20.0, 30.0};

		return std::make_shared<GTouchable>(gopt, "readout", identity, dimensions);
	}

	// create fake gtouchable for testing purposes, using sector and fake dimensions
	static std::shared_ptr<GTouchable> create(const std::shared_ptr<GLogger>& logger) {
		int         touchableNumber = globalGTouchableCounter.fetch_add(1, std::memory_order_relaxed);
		int         sector          = (touchableNumber % 6) + 1;
		int         paddle          = (touchableNumber % 20) + 1;
		std::string identity        = "sector: " + std::to_string(sector) + ", paddle: " + std::to_string(paddle);
		const auto& dimensions      = {10.0, 20.0, 30.0};

		return std::make_shared<GTouchable>(logger, "readout", identity, dimensions);
	}

private:
	GTouchableType gType; ///< The type of the touchable element.
	std::vector<GIdentifier> gidentity; ///< Unique identifiers for the detector element.
	int trackId; ///< Track id (used in flux and dosimeter types). Assigned in sensitiveDetector::ProcessHit
	double eMultiplier; ///< Energy multiplier for energy sharing. Set by processGTouchable in the digitization plugin. Defaulted to 1. Used to share energy / create new hits.
	int stepTimeAtElectronicsIndex;
	///< Used to determine if a hit is within an existing detector readout electronic time window. Set by the digitization plugin using the readout specs.
	std::vector<double> detectorDimensions; ///< Physical dimensions of the detector element. Saved to be used in GDynamicDigitization if needed


	/// Overloaded output operator for GTouchable.
	friend std::ostream& operator<<(std::ostream& stream, const GTouchable& gtouchable);

	/// Static thread-safe event counter - used for testing only
	static std::atomic<int> globalGTouchableCounter;
};
