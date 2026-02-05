#pragma once

// geant4
#include "G4VSensitiveDetector.hh"

// gemc
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "gbase.h"

/**
 * @file gsd.h
 * @ingroup gsd_module
 * @brief Declares the GSensitiveDetector class and module-level constants.
 */

/**
 * @name GSensitiveDetector error codes
 * @ingroup gsd_module
 *
 * These error codes are used for logging and diagnostics within GSensitiveDetector.
 * @{
 */

/**
 * @brief Error code used when a required dynamic plugin-dependent resource is missing.
 *
 * This module logs this code when it cannot retrieve a registered GTouchable for a Geant4 volume name.
 */
constexpr int ERR_DYNAMICPLUGINNOTFOUND = 2001;

/**
 * @brief Error code used when a hit is expected to exist but cannot be found in the current hit collection.
 */
constexpr int ERR_HITNOTFOUNDINCOLLECTION = 2002;

/**
 * @brief Error code used when the Geant4 hits collection is unexpectedly missing.
 */
constexpr int ERR_NOCOLLECTION = 2003;

/**
 * @brief Logger name used by this module.
 *
 * This string is passed to the logging system via the module options.
 */
constexpr const char* GSENSITIVE_LOGGER = "gsd";

/** @} */

namespace gsensitivedetector {

/**
 * @ingroup gsd_module
 * @brief Defines the module options for GSensitiveDetector.
 *
 * The returned GOptions instance is initialized with the module logger name (\c "gsd").
 * This function is intentionally lightweight so it can be used during configuration assembly.
 *
 * @return A GOptions instance associated with this module logger.
 */
inline GOptions defineOptions() { return GOptions(GSENSITIVE_LOGGER); }

} // namespace gsensitivedetector

/**
 * @ingroup gsd_module
 * @brief Convenience alias for the Geant4 hits collection used by this module.
 *
 * GHitsCollection is the concrete Geant4 hits container used to store pointers to GHit objects.
 * The underlying type is \c G4THitsCollection<GHit>.
 */
using GHitsCollection = G4THitsCollection<GHit>;

/**
 * @class GSensitiveDetector
 * @ingroup gsd_module
 * @brief Thread-local sensitive detector bridging Geant4 steps to GEMC hits via digitization plugins.
 *
 * GSensitiveDetector implements the \c G4VSensitiveDetector interface and participates in the Geant4
 * event loop through:
 * - \ref GSensitiveDetector::Initialize "Initialize()" : per-event initialization (hits collection creation/registration).
 * - \ref GSensitiveDetector::ProcessHits "ProcessHits()" : per-step processing (create/update hits).
 * - \ref GSensitiveDetector::EndOfEvent "EndOfEvent()" : end-of-event hook.
 *
 * ### Responsibilities
 * - Own a per-event Geant4 hits collection (GHitsCollection) and register it into \c G4HCofThisEvent.
 * - Use the assigned GDynamicDigitization routine to:
 *   - decide whether a step should be ignored,
 *   - map the Geant4 step to one or more logical touchables,
 *   - read the HitBitSet describing which information is stored in hits.
 * - Track whether a touchable has already produced a hit in the current event and either create a new GHit
 *   or update an existing one.
 *
 * ### Threading and lifetime
 * - Instances are intended to be thread-local.
 * - The digitization routine must be assigned with assign_digi_routine() before processing begins.
 * - Touchables are registered once at construction/setup time and then treated as read-only during event processing.
 */
class GSensitiveDetector : public GBase<GSensitiveDetector>, public G4VSensitiveDetector {

public:
	/**
	 * @brief Constructs a sensitive detector instance for a given detector name.
	 *
	 * The constructor initializes the logging base and forwards the sensitive detector name to \c G4VSensitiveDetector.
	 * It also initializes the Geant4 collection name vector with a module-specific collection name based on @p sdName.
	 *
	 * @param sdName    Name of the sensitive detector instance (used by Geant4 and for collection naming).
	 * @param goptions  Shared options object used to configure logging and module behavior.
	 */
	GSensitiveDetector(const std::string&               sdName,
	                   const std::shared_ptr<GOptions>& goptions);

	/**
	 * @brief Per-event initialization hook called by Geant4.
	 *
	 * This method:
	 * - retrieves the HitBitSet from the digitization routine readout specifications,
	 * - clears the per-event touchable cache,
	 * - allocates a new Geant4 hits collection (GHitsCollection),
	 * - registers it into the event hit container (\c G4HCofThisEvent).
	 *
	 * @param g4hc Geant4 event hit container that will own/track the hits collection for this event.
	 */
	void Initialize(G4HCofThisEvent* g4hc) override;

	/**
	 * @brief Processes a Geant4 step and creates or updates hits in the current hits collection.
	 *
	 * The digitization routine can choose to skip hits based on deposited energy and can transform the input touchable
	 * into one or more logical touchables. For each resulting touchable, this method either:
	 * - creates a new GHit and inserts it in the hits collection, or
	 * - locates an existing GHit and appends step information according to the HitBitSet.
	 *
	 * @param thisStep Geant4 step being processed.
	 * @param g4th     Geant4 touchable history (not used by this implementation).
	 * @return \c true to indicate the step was handled (even if skipped by policy).
	 */
	G4bool ProcessHits(G4Step* thisStep, G4TouchableHistory* g4th) override;

	/**
	 * @brief End-of-event hook called by Geant4.
	 *
	 * This method is invoked after all steps have been processed for the event. The Geant4 event system will own the
	 * hits collection registered during Initialize().
	 *
	 * @param g4HitCollection Geant4 event hit container (not used by this implementation).
	 */
	void EndOfEvent(G4HCofThisEvent* g4HitCollection) override;

	/**
	 * @brief Assigns the digitization routine used to interpret steps and define hit content.
	 *
	 * The assigned routine is expected to remain valid for the lifetime of this sensitive detector instance.
	 *
	 * @param digi_routine Digitization routine responsible for readout specs and step processing.
	 */
	void assign_digi_routine(std::shared_ptr<GDynamicDigitization> digi_routine) { digitization_routine = digi_routine; }

private:
	/**
	 * @brief Thread-local digitization routine used by this sensitive detector.
	 *
	 * The routine provides:
	 * - readout specifications (including HitBitSet),
	 * - the decision policy to skip steps,
	 * - the touchable processing logic that can split/transform touchables.
	 */
	std::shared_ptr<GDynamicDigitization> digitization_routine;

	/**
	 * @brief Bitset describing which hit information is stored by GHit for this event.
	 *
	 * This is loaded at event initialization from the digitization routine readout specifications so that
	 * ProcessHits() does not repeatedly retrieve it.
	 */
	HitBitSet gHitBitSet;

	/**
	 * @brief Map of volume name to registered GTouchable.
	 *
	 * There is one entry per gvolume. The map is populated during detector construction and is used to quickly
	 * retrieve the GTouchable corresponding to the Geant4 volume in which a step occurs.
	 */
	std::map<std::string, std::shared_ptr<GTouchable>> gTouchableMap;

	/**
	 * @brief Retrieves the registered GTouchable corresponding to the volume of @p thisStep.
	 *
	 * The map entry is expected to exist because the registry is populated at detector construction time.
	 * If the entry is missing, the function logs an error.
	 *
	 * @param thisStep Geant4 step whose pre-step point defines the containing volume.
	 * @return Shared pointer to the registered GTouchable for the volume.
	 */
	inline std::shared_ptr<GTouchable> getGTouchable(const G4Step* thisStep) {
		std::string vname = thisStep->GetPreStepPoint()->GetTouchable()->GetVolume()->GetName();

		auto it = gTouchableMap.find(vname);
		if (it != gTouchableMap.end()) { return it->second; }
		// If not found, log an error. The calling code assumes a valid pointer.
		log->error(ERR_DYNAMICPLUGINNOTFOUND, "GTouchable for volume " + vname + " not found in gTouchableMap");
	}

	/**
	 * @brief Per-event cache of touchables already associated with a hit.
	 *
	 * This vector is cleared at the start of each event and is used to decide whether a processed touchable
	 * should create a new hit or update an existing one.
	 */
	std::vector<GTouchable> touchableVector;

	/**
	 * @brief Determines whether @p thisTouchable is new in the current event and updates the per-event cache.
	 *
	 * If @p thisTouchable is not present in touchableVector, it is appended and the function returns \c true.
	 * Otherwise, the function returns \c false.
	 *
	 * @param thisTouchable Touchable to test and potentially record.
	 * @return \c true if the touchable was not previously seen in this event, otherwise \c false.
	 */
	bool isThisANewTouchable(const std::shared_ptr<GTouchable>& thisTouchable);

	/**
	 * @brief Pointer to the current event hits collection.
	 *
	 * The collection is allocated during Initialize() and registered with the event container. It is then used
	 * by ProcessHits() to insert and retrieve hits.
	 */
	GHitsCollection* gHitsCollection;

	/**
	 * @brief Retrieves an existing hit in the current hit collection matching @p gtouchable.
	 *
	 * This method iterates over the hits collection and compares each hit's GTouchable to @p gtouchable.
	 *
	 * @param gtouchable Touchable identifying the hit to retrieve.
	 * @return Pointer to the matching hit, if found.
	 */
	GHit* getHitInHitCollectionUsingTouchable(const std::shared_ptr<GTouchable>& gtouchable);

public:
	/**
	 * @brief Registers a GTouchable for a given gvolume name into the internal lookup map.
	 *
	 * This function is typically used during detector construction (e.g., from GDetectorConstruction) to populate
	 * the mapping between volume names and their corresponding touchables.
	 *
	 * @param name Volume name (key) used later to locate the touchable during ProcessHits().
	 * @param gt   Touchable instance associated with @p name.
	 */
	inline void registerGVolumeTouchable(const std::string& name, std::shared_ptr<GTouchable> gt) {
		log->info(2, "Registering touchable gvolume <" + name + "> with value: " + gt->getIdentityString());

		// Store the GTouchable in the map; this module retains a shared ownership reference.
		gTouchableMap[name] = gt;
	}
};
