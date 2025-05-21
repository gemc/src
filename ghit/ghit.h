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

/**
 * \class GHit
 * \brief Represents a hit in the detector.
 *
 * This class is derived from G4VHit and is used to store information about
 * a detector hit. It collects standard hit data (energy, time, positions) as well
 * as additional information based on a configurable HitBitSet.
 */
class GHit : public G4VHit {
public:
	/**
	 * \brief Constructor for GHit.
	 * \param gt Pointer to the GTouchable that produced the hit.
	 * \param thisStep Pointer to the G4Step associated with the hit.
	 * \param hbs Bitset selecting which hit information to record.
	 * \param cScheme Color schema for visualization (default is "default").
	 */
	GHit(GTouchable* gt, HitBitSet hbs, const G4Step* thisStep = nullptr, std::string cScheme = "default");

	/**
	 * \brief Destructor for GHit.
	 */
	~GHit() override = default;

	/**
	 * \brief Overloaded new operator using G4Allocator.
	 * \return Pointer to allocated memory.
	 */
	inline void* operator new(size_t);

	/**
	 * \brief Overloaded delete operator using G4Allocator.
	 */
	inline void operator delete(void*);

	/**
	 * \brief Draws the hit using Geant4 visualization.
	 */
	void Draw() override;

	/**
	 * @brief Compare two ghits
	 * @param hit
	 * @return Returns true if this gtouchable is the same as the one in the hit.
	 */
	bool is_same_hit(GHit* hit);

private:
	G4Colour colour_touch, colour_hit, colour_passby;

	bool setColorSchema();

	std::string colorSchema;

	// GTouchable saved here so it can be used in the overloaded == function
	GTouchable* gtouchable;

	// hit data, selected by HitBitSet, to be collected for each step
	// always present:
	std::vector<double>        edeps, times;
	std::vector<G4ThreeVector> globalPositions;
	std::vector<G4ThreeVector> localPositions;

	// bit 1
	std::vector<int>         pids;
	std::vector<double>      Es;
	std::vector<std::string> processNames;

	// bit 2
	std::vector<double> stepSize;

	// calculated/single quantities
	std::optional<double> totalEnergyDeposited;
	double                averageTime;
	G4ThreeVector         avgGlobalPosition;
	G4ThreeVector         avgLocalPosition;
	std::string           processName;

	// build hit information based on the bit index and the touchable
	bool addHitInfosForBitIndex(size_t bitIndex, bool test, const G4Step* thisStep);

public:
	// inline getters for hit information:
	/**
	   * \brief Gets the energy depositions from each step.
	   * \return A vector of energy deposition values.
	   */
	[[nodiscard]] inline std::vector<double> getEdeps() const { return edeps; }

	/**
	 * \brief Gets the time stamps for each step.
	 * \return A vector of time values.
	 */
	[[nodiscard]] inline std::vector<double> getTimes() const { return times; }

	/**
	 * \brief Gets the global positions recorded for the hit.
	 * \return A vector of G4ThreeVector representing global positions.
	 */
	[[nodiscard]] inline std::vector<G4ThreeVector> getGlobalPositions() const { return globalPositions; }

	/**
	  * \brief Gets the local positions recorded for the hit.
	  * \return A vector of G4ThreeVector representing local positions.
	  */
	[[nodiscard]] inline std::vector<G4ThreeVector> getLocalPositions() const { return localPositions; }

	/**
	 * \brief Gets the particle IDs recorded for the hit.
	 * \return A vector of integer particle IDs.
	 */
	[[nodiscard]] inline std::vector<int> getPids() const { return pids; }

	/**
	 * \brief Returns the first particle ID.
	 * \note This method assumes that the 'pids' vector is not empty.
	 * \return The first particle ID.
	 */
	[[nodiscard]] inline int getPid() const { return pids.front(); }

	/**
	 * \brief Gets the energy values recorded in bit 1.
	 * \return A vector of energy values.
	 */
	[[nodiscard]] inline std::vector<double> getEs() const { return Es; }

	/**
	 * \brief Returns the first energy value.
	 * \note Assumes Es is non-empty.
	 * \return The first energy value.
	 */
	[[nodiscard]] inline double getE() const { return Es.front(); }

	/**
	 * \brief Gets the process name associated with the hit.
	 * \return The process name string.
	 */
	[[nodiscard]] inline std::string getProcessName() const { return processName; }

	/**
   * \brief Returns the GTouchable associated with the hit.
   * \return Pointer to the GTouchable.
   */
	[[nodiscard]] inline GTouchable* getGTouchable() const { return gtouchable; }

	/**
	 * \brief Returns the detector element identity.
	 * \return A vector of GIdentifier.
	 */
	[[nodiscard]] inline std::vector<GIdentifier> getGID() const { return gtouchable->getIdentity(); }

	/**
     * \brief Returns the dimensions of the detector element.
     * \return A vector of double containing the dimensions.
     */
	[[nodiscard]] inline std::vector<double> getDetectorDimensions() const { return gtouchable->getDetectorDimensions(); }

	// calculated quantities
	void calculateInfosForBit(int bit);

	double getTotalEnergyDeposited();

	double getAverageTime();

	G4ThreeVector getAvgLocalPosition();

	G4ThreeVector getAvgGlobaPosition();

	// gemc api
	// build hit information based on the G4Step
	void addHitInfosForBitset(HitBitSet hbs, const G4Step* thisStep);


	/**
	 * \brief Returns the touchable identity values as integers.
	 *
	 * Iterates over the GIdentifier vector from the associated GTouchable.
	 *
	 * \return A vector of integer identity values.
	 */
	std::vector<int> getTTID();

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
