#ifndef  GTOUCHABLE_H
#define  GTOUCHABLE_H 1

#include "glogger.h"

// c++
#include <vector>
#include <string>
#include <iostream>

// - readout: electronic Time Window is the discriminating factor.
//   parameters and hitBitSet determined by defineReadoutSpecs in the plugin
// - flux: track id is the discriminating factor, standard true infos variable
// - particleCounter: no other discriminating factors, standard true infos variable
// - dosimeter: track id is the discriminating factor, radiation digitization



/**
 * @brief Enumeration representing the type of a GTouchable element.
 *
 * Defines different types of sensitive detector elements.
 */
enum GTouchableType {
	readout,          ///< Electronic readout with time window discrimination.
	flux,             ///< Track-based discrimination.
	particleCounter,  ///< No additional discriminating factors.
	dosimeter         ///< Radiation digitization using track id.
};

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
	GIdentifier(std::string n, int v) : idName{n}, idValue{v} {}

	/**
	 * @brief Equality operator comparing only the identifier value.
	 * @param gid The GIdentifier to compare.
	 * @return True if the identifier values are equal.
	 */
	bool operator==(const GIdentifier &gid) const { return this->idValue == gid.idValue; }

	/**
	 * @brief Gets the identifier name.
	 * @return The identifier name.
	 */
	inline std::string getName() const { return idName; }

	/**
	 * @brief Gets the identifier value.
	 * @return The identifier value.
	 */
	inline int getValue() const { return idValue; }



private:
	string idName;
	int idValue;

	/// Overloaded output operator for GIdentifier.
	friend std::ostream &operator<<(std::ostream &stream, const GIdentifier &gidentifier);

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
class GTouchable {

public:
	/**
	* @brief Constructs a GTouchable from digitization and identifier strings.
	*
	* Called in GDetectorConstruction::ConstructSDandField to register a new
	* GTouchable in the sensitive detector map.
	*
	* @param digitization The digitization type as a string.
	* @param gidentityString The string specifying the gidentity (e.g., "sector: 2, layer: 4, wire: 33").
	* @param dimensions The physical dimensions of the detector element.
	* @param logger Pointer to the GLogger instance for logging messages.
	*/
	GTouchable(const std::string &digitization, const std::string &gidentityString, const std::vector<double> &dimensions, GLogger * const logger);


	/**
	* @brief Copy constructor for updating the electronics time index.
	*
	* Used when the step time index of the hit is different from that of the GTouchable.
	*
	* @param baseGT Pointer to the base GTouchable.
	* @param newTimeIndex The new electronics time index.
	*/
	GTouchable(const GTouchable *baseGT, int newTimeIndex);


	~GTouchable() {
		log->debug(DESTRUCTOR, "GTouchable");
	}

	/**
	* @brief Equality operator comparing two GTouchable objects.
	*
	* Comparison is based on the gidentity vector and, if needed, on a type-specific property.
	*
	* @param gtouchable The GTouchable to compare with.
	* @return True if the objects are considered equal.
	*/
	bool operator==(const GTouchable &gtouchable) const;

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
	inline float getEnergyMultiplier() const { return eMultiplier; }

	/**
 	* @brief Assigns the step time index used in electronics.
	* @param timeIndex The new time index.
 	*/
	inline void assignStepTimeAtElectronicsIndex(int timeIndex) { stepTimeAtElectronicsIndex = timeIndex; }

	/**
	* @brief Gets the electronics time index.
	* @return The step time index.
	*/
	inline int getStepTimeAtElectronicsIndex() const { return stepTimeAtElectronicsIndex; }

	/**
	* @brief Returns the identifier vector.
	* @return A vector of GIdentifier objects.
	*/
	inline const std::vector<GIdentifier> getIdentity() const { return gidentity; }

	/**
	* @brief Returns the detector dimensions.
	* @return A vector containing the dimensions.
	*/
	inline const std::vector<double> getDetectorDimensions() const { return detectorDimensions; }

	/**
	 * @brief Checks if the GTouchable is found in a vector of GTouchable objects.
	 * @param v The vector of GTouchable objects.
	 * @return True if the GTouchable is found in the vector.
	 */

	bool exists_in_vector(const std::vector<GTouchable> &v) const {
		for (const auto &gt : v) {
			if (*this == gt) {
				log->info("GTouchable", this, " exists in vector.");
				return true;
			}
		}
		log->info("GTouchable", this, " does not exist in vector.");

		return false;
	}

private:
	GLogger * const log;                           ///< Logger instance (assumed to be managed externally; consider using a smart pointer if ownership semantics change).
	GTouchableType gType;                   ///< The type of the touchable element.
	std::vector <GIdentifier> gidentity;     ///< Unique identifiers for the detector element.
	int trackId;                            ///< Track id (used in flux and dosimeter types). Assigned in sensitiveDetector::ProcessHit
	float eMultiplier;                      ///< Energy multiplier for energy sharing. Set by processGTouchable in the digitization plugin. Defaulted to 1. Used to share energy / create new hits.
	int stepTimeAtElectronicsIndex;         ///< Used to determine if a hit is within an existing detector readout electronic time window. Set by the digitization plugin using the readout specs.
	std::vector<double> detectorDimensions; ///< Physical dimensions of the detector element. Saved to be used in GDynamicDigitization if needed


	/// Overloaded output operator for GTouchable.
	friend std::ostream &operator<<(std::ostream &stream, const GTouchable &gtouchable);

};

#endif
