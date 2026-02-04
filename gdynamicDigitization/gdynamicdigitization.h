#pragma once

#include "greadoutSpecs.h"
#include "gfactory_options.h"

// gemc
#include "gfactory.h"
#include "gtouchable.h"
#include "ghit.h"
#include "gDigitizedData.h"
#include "gTrueInfoData.h"
#include "gtranslationTable.h"
#include "gdynamicdigitization_options.h"

// c++
#include <utility>
#include <vector>
#include <bitset>
#include <map>
#include <string>
#include <optional>

// geant4
#include "G4Step.hh"

/**
 * \class GTouchableModifiers
 * \brief Helper container for representing touchable “modifier” information.
 *
 * Some digitization routines need to create one or more derived touchables from a single
 * touchable, weighted by occupancy, energy, time, or similar criteria.
 *
 * This helper stores modifier payloads in one of two mutually exclusive representations:
 * - weights-only : flat pairs (id, weight)
 * - weights-and-times : flat triplets (id, weight, time)
 *
 * \note
 * Only one representation is expected to be filled for a given instance.
 */
class GTouchableModifiers
{
public:
	/**
	 * \brief Constructs the container and declares supported touchable names.
	 *
	 * Declaring touchable names up-front ensures the internal maps contain keys for all
	 * expected touchables.
	 *
	 * \param touchableNames List of touchable names for which modifiers may be recorded.
	 */
	explicit GTouchableModifiers(const std::vector<std::string>& touchableNames);

private:
	/**
	 * \brief Map holding flat (id, weight) pairs for each touchable.
	 *
	 * Layout for one touchable is:
	 * \code
	 * [id0, weight0, id1, weight1, ...]
	 * \endcode
	 */
	std::map<std::string, std::vector<double>> modifierWeightsMap;

	/**
	 * \brief Map holding flat (id, weight, time) triplets for each touchable.
	 *
	 * Layout for one touchable is:
	 * \code
	 * [id0, weight0, time0, id1, weight1, time1, ...]
	 * \endcode
	 */
	std::map<std::string, std::vector<double>> modifierWeightsAndTimesMap;

public:
	/**
	 * \brief Inserts a new (id, weight) pair for the specified touchable.
	 *
	 * \param touchableName Name of the touchable to modify.
	 * \param idValue Identifier value associated with the modifier entry.
	 * \param weight Unnormalized weight (caller may later normalize with assignOverallWeight()).
	 */
	void insertIdAndWeight(const std::string& touchableName, int idValue, double weight);

	/**
	 * \brief Inserts a new (id, weight, time) triplet for the specified touchable.
	 *
	 * \param touchableName Name of the touchable to modify.
	 * \param idValue Identifier value associated with the modifier entry.
	 * \param weight Unnormalized weight (caller may later normalize with assignOverallWeight()).
	 * \param time Time associated with this modifier entry (time unit follows project conventions).
	 */
	void insertIdWeightAndTime(const std::string& touchableName, int idValue, double weight, double time);

	/**
	 * \brief Normalizes modifier weights by dividing by a provided total.
	 *
	 * - For weights-only vectors, normalizes every second element.
	 * - For weights-and-times vectors, normalizes the weight element of each triplet.
	 *
	 * \param touchableName Name of the touchable whose modifiers are to be normalized.
	 * \param totalWeight Denominator for normalization (must be non-zero).
	 */
	void assignOverallWeight(const std::string& touchableName, double totalWeight);

	/**
	 * \brief Returns whether the container currently reports weights-only modifiers.
	 *
	 * \return true if modifierWeightsMap is non-empty.
	 *
	 * \note
	 * This is a coarse check: it reports whether the map is non-empty, not whether a
	 * specific touchable has entries.
	 */
	[[nodiscard]] inline bool isWeightsOnly() const { return !modifierWeightsMap.empty(); }

	/**
	 * \brief Returns the weights vector for a touchable.
	 *
	 * \param touchableName Touchable name key.
	 * \return A copy of the internal flat vector containing (id, weight) pairs.
	 *
	 * \warning
	 * This function will crash if the key was not declared in the constructor.
	 */
	inline std::vector<double> getModifierWeightsVector(const std::string& touchableName) {
		return modifierWeightsMap[touchableName];
	}

	/**
	 * \brief Returns the weights-and-times vector for a touchable.
	 *
	 * \param touchableName Touchable name key.
	 * \return A copy of the internal flat vector containing (id, weight, time) triplets.
	 *
	 * \warning
	 * This function will crash if the key was not declared in the constructor.
	 */
	inline std::vector<double> getModifierWeightsAndTimeVector(const std::string& touchableName) {
		return modifierWeightsAndTimesMap[touchableName];
	}
};

/**
 * \class GDynamicDigitization
 * \brief Abstract base class for dynamically loaded digitization plugins.
 *
 * Plugins typically implement:
 * - \ref GDynamicDigitization::defineReadoutSpecs "defineReadoutSpecs()"
 * - \ref GDynamicDigitization::digitizeHit "digitizeHit()"
 *
 * They may additionally override:
 * - \ref GDynamicDigitization::processStepTime "processStepTime()"
 * - \ref GDynamicDigitization::processTouchable "processTouchable()"
 * - \ref GDynamicDigitization::collectTrueInformation "collectTrueInformation()"
 * - \ref GDynamicDigitization::loadConstants "loadConstants()"
 * - \ref GDynamicDigitization::loadTT "loadTT()"
 *
 */
class GDynamicDigitization : public GBase<GDynamicDigitization>
{
public:
	/**
	 * \brief Constructs the digitization base object.
	 *
	 * \param g Options used by this plugin instance.
	 */
	explicit GDynamicDigitization(const std::shared_ptr<GOptions>& g) : GBase(g, GDIGITIZATION_LOGGER) {
	}

	/// Virtual destructor.
	virtual ~GDynamicDigitization() = default;

	/**
	 * \brief Computes the time associated with a simulation step for electronics binning.
	 *
	 * This wrapper logs, checks that options are configured, and delegates to
	 * processStepTimeImpl().
	 *
	 * \param gTouchID Touchable associated with the current step.
	 * \param thisStep Pointer to the current \c G4Step.
	 * \return Time value used for electronics time binning (unit follows project conventions).
	 */
	[[nodiscard]] double processStepTime(const std::shared_ptr<GTouchable>& gTouchID,
	                                     [[maybe_unused]] G4Step*           thisStep) {
		check_if_log_defined();
		log->debug(NORMAL, FUNCTION_NAME);
		return processStepTimeImpl(gTouchID, thisStep);
	}

	/**
	 * \brief Implementation hook for step time computation.
	 *
	 * Default behavior returns the post-step global time from \c G4Step.
	 *
	 * \param gTouchID Touchable associated with the current step.
	 * \param thisStep Pointer to the current \c G4Step.
	 * \return Time value used for electronics time binning.
	 */
	[[nodiscard]] virtual double processStepTimeImpl(const std::shared_ptr<GTouchable>& gTouchID,
	                                                 [[maybe_unused]] G4Step*           thisStep);

	/**
	 * \brief Processes a touchable based on the current step and readout specs.
	 *
	 * The default behavior bins the step time into an electronics time-cell index using
	 * readoutSpecs->timeCellIndex(). If the index changes relative to the current touchable,
	 * a second touchable instance is created.
	 *
	 * \param gtouchable Input touchable for this step.
	 * \param thisStep Pointer to the current \c G4Step.
	 * \return One or more touchables representing the time-binned view of the input.
	 */
	[[nodiscard]] std::vector<std::shared_ptr<GTouchable>> processTouchable(
		std::shared_ptr<GTouchable> gtouchable, G4Step* thisStep) {
		check_if_log_defined();
		log->debug(NORMAL, FUNCTION_NAME);
		return processTouchableImpl(std::move(gtouchable), thisStep);
	}

	/**
	 * \brief Implementation hook for touchable processing.
	 *
	 * \param gtouchable Input touchable for this step.
	 * \param thisStep Pointer to the current \c G4Step.
	 * \return One or more touchables representing the time-binned view of the input.
	 */
	[[nodiscard]] virtual std::vector<std::shared_ptr<GTouchable>> processTouchableImpl(
		std::shared_ptr<GTouchable> gtouchable, G4Step* thisStep);

	/**
	 * \brief Applies touchable modifiers.
	 *
	 * Wrapper that logs/checks and delegates to processGTouchableModifiersImpl().
	 *
	 * \param gTouchID Input touchable to modify.
	 * \param gmods Modifier container describing how to modify touchables.
	 * \return A vector of modified touchables (possibly empty).
	 */
	[[nodiscard]] std::vector<std::shared_ptr<GTouchable>> processGTouchableModifiers(
		const std::shared_ptr<GTouchable>& gTouchID, const GTouchableModifiers& gmods) {
		check_if_log_defined();
		log->debug(NORMAL, FUNCTION_NAME);
		return processGTouchableModifiersImpl(gTouchID, gmods);
	}

	/**
	 * \brief Implementation hook for touchable modifier application.
	 *
	 * Default implementation returns an empty vector (no modifiers applied).
	 *
	 * \param gTouchID Input touchable.
	 * \param gmods Modifier container.
	 * \return Modified touchables (default: empty).
	 */
	virtual std::vector<std::shared_ptr<GTouchable>> processGTouchableModifiersImpl(
		[[maybe_unused]] const std::shared_ptr<GTouchable>& gTouchID,
		[[maybe_unused]] const GTouchableModifiers&         gmods);

	/**
	 * \brief Collects standardized “true hit information” into a GTrueInfoData record.
	 *
	 * Wrapper that logs/checks and delegates to collectTrueInformationImpl().
	 *
	 * \param ghit Input hit from the sensitive detector.
	 * \param hitn Sequential hit index.
	 * \return Newly created true-information record.
	 */
	[[nodiscard]] std::unique_ptr<GTrueInfoData> collectTrueInformation(GHit* ghit, size_t hitn) {
		check_if_log_defined();
		log->info(2, "GDynamicDigitization::collect true information for hit number ", hitn, " with size ",
		          ghit->nsteps(), " steps");
		return collectTrueInformationImpl(ghit, hitn);
	}

	/**
	 * \brief Implementation hook for true-information collection.
	 *
	 * \param ghit Input hit.
	 * \param hitn Sequential hit index.
	 * \return Newly created true-information record.
	 */
	[[nodiscard]] virtual std::unique_ptr<GTrueInfoData> collectTrueInformationImpl(GHit* ghit, size_t hitn);

	/**
	 * \brief Digitizes a hit into a GDigitizedData record.
	 *
	 * Wrapper that logs/checks and delegates to digitizeHitImpl().
	 *
	 * \param ghit Input hit from the sensitive detector.
	 * \param hitn Sequential hit index.
	 * \return Newly created digitized record (or nullptr if not implemented).
	 */
	[[nodiscard]] std::unique_ptr<GDigitizedData> digitizeHit(GHit* ghit, [[maybe_unused]] size_t hitn) {
		check_if_log_defined();
		log->info(2, "GDynamicDigitization::digitize  hit number ", hitn, " with size ", ghit->nsteps(), " steps");
		return digitizeHitImpl(ghit, hitn);
	}

	/**
	 * \brief Implementation hook for hit digitization.
	 *
	 * Default implementation returns nullptr to indicate "not implemented".
	 *
	 * \param ghit Input hit.
	 * \param hitn Sequential hit index.
	 * \return Newly created digitized record, or nullptr.
	 */
	[[nodiscard]] virtual std::unique_ptr<GDigitizedData> digitizeHitImpl(
		[[maybe_unused]] GHit* ghit, [[maybe_unused]] size_t hitn) { return nullptr; }

	/**
	 * \brief Loads digitization constants (calibration/configuration).
	 *
	 * Wrapper that logs/checks and delegates to loadConstantsImpl().
	 *
	 * \param runno Run number.
	 * \param variation Variation string.
	 * \return true on success.
	 */
	[[nodiscard]] bool loadConstants([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) {
		check_if_log_defined();
		log->debug(NORMAL, "GDynamicDigitization::load constants");
		return loadConstantsImpl(runno, variation);
	}

	/**
	 * \brief Implementation hook for constant loading.
	 *
	 * Default implementation does nothing and returns true.
	 *
	 * \param runno Run number.
	 * \param variation Variation string.
	 * \return true on success.
	 */
	virtual bool loadConstantsImpl([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) {
		return true;
	}

	/**
	 * \brief Loads the translation table (identity -> electronics address).
	 *
	 * Wrapper that logs/checks and delegates to loadTTImpl().
	 *
	 * \param runno Run number.
	 * \param variation Variation string.
	 * \return true on success.
	 */
	[[nodiscard]] bool loadTT([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) {
		check_if_log_defined();
		log->debug(NORMAL, "GDynamicDigitization::load Translation Table for run ", runno, " with variation ",
		           variation);
		return loadTTImpl(runno, variation);
	}

	/**
	 * \brief Implementation hook for translation table loading.
	 *
	 * Default implementation does nothing and returns true.
	 *
	 * \param runno Run number.
	 * \param variation Variation string.
	 * \return true on success.
	 */
	virtual bool loadTTImpl([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) { return true; }

	/**
	 * \brief Adds hardware-level time/charge and address fields to a digitized record.
	 *
	 * This routine inserts:
	 * - crate/slot/channel from the translation table
	 * - TIMEATELECTRONICS and CHARGEATELECTRONICS
	 *
	 * \param time Time value (time unit follows project conventions; often ns).
	 * \param q Charge value (ADC-like integer).
	 * \param ghit Input hit providing the translation-table id.
	 * \param gdata Digitized record to augment (caller retains ownership).
	 */
	void chargeAndTimeAtHardware(int time, int q, const GHit* ghit, GDigitizedData& gdata);

	/**
	 * \brief Initializes readout specifications.
	 *
	 * Wrapper that logs/checks and delegates to defineReadoutSpecsImpl().
	 *
	 * \return true on success.
	 */
	[[nodiscard]] bool defineReadoutSpecs() {
		check_if_log_defined();
		log->debug(NORMAL, "GDynamicDigitization::define readout specs");
		return defineReadoutSpecsImpl();
	};

	/**
	 * \brief Implementation hook to define readout specs.
	 *
	 * Must be implemented by derived classes.
	 *
	 * \return true on success.
	 */
	virtual bool defineReadoutSpecsImpl() = 0;

	/// Readout specs are created during initialization and treated as immutable.
	std::shared_ptr<const GReadoutSpecs> readoutSpecs;

	/// Translation table is typically loaded during initialization and treated as immutable.
	std::shared_ptr<const GTranslationTable> translationTable;

	/**
	 * \brief Dynamically instantiates a plugin instance from a dynamic library.
	 *
	 * This function looks up the \c GDynamicDigitizationFactory symbol and calls it.
	 *
	 * \param h Dynamic library handle.
	 * \param g Options to pass to the plugin instance.
	 * \return Newly created plugin instance, or nullptr on failure.
	 */
	static GDynamicDigitization* instantiate(const dlhandle h, std::shared_ptr<GOptions> g) {
		if (!h) return nullptr;
		using fptr = GDynamicDigitization* (*)(std::shared_ptr<GOptions>);

		auto sym = dlsym(h, "GDynamicDigitizationFactory");
		if (!sym) return nullptr;

		auto func = reinterpret_cast<fptr>(sym);
		return func(g);
	}

	/**
	 * \brief Decides whether a hit should be skipped based on deposited energy.
	 *
	 * Default behavior skips hits with exactly zero energy if the \c recordZeroEdep option
	 * is disabled.
	 *
	 * \param energy Total deposited energy for the hit.
	 * \return true if the hit should be skipped.
	 */
	virtual bool decisionToSkipHit(double energy) {
		if (energy == 0 && !recordZeroEdep) {
			return true;
		}
		return false;
	}

	/**
	 * \brief Sets the options pointer required by the digitization base.
	 *
	 * \param g Options object to store for later use.
	 *
	 * \warning
	 * A future refactor may remove this pattern; currently it is required for legacy flows.
	 */
	// TODO: REMOVE THIS EVERYWHERE also remove check_if_log_defined
	void set_loggers(const std::shared_ptr<GOptions>& g) { gopts = g; }

private:
	/// When false, hits with exactly zero deposited energy may be skipped.
	bool recordZeroEdep = false;

protected:
	/// Options used by the digitization plugin instance.
	std::shared_ptr<GOptions> gopts;

	/**
	 * \brief Ensures options/logging are configured before plugin methods run.
	 *
	 * If options are not set, prints an error message and exits.
	 */
	void check_if_log_defined() const {
		if (gopts == nullptr) {
			std::cerr << KRED
				<< "Fatal Error: GDynamicDigitization: goption is not set for this plugin or one of the loggers is null."
				<< std::endl;
			std::cerr << "The set_loggers function needs to be called." << std::endl;
			std::cerr << "For example: dynamicRoutines[\"ctof\"]->set_loggers(gopts);" << std::endl;
			std::cerr << RST << std::endl;
			exit(1);
		}
	}
};

namespace gdynamicdigitization {
using dRoutinesMap = std::unordered_map<std::string, std::shared_ptr<GDynamicDigitization>>;

/**
 * \brief Loads a single dynamic routine and returns it.
 *
 * \param plugin_name Name of the plugin library/object.
 * \param gopts Shared options.
 * \return Loaded plugin instance.
 */
inline std::shared_ptr<GDynamicDigitization> load_dynamicRoutine(const std::string&               plugin_name,
                                                                 const std::shared_ptr<GOptions>& gopts) {
	GManager manager(gopts);
	return manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>(plugin_name, gopts);
}

/**
 * \brief Loads multiple dynamic routines and returns an immutable shared map.
 *
 * The returned map is shared and treated as immutable to prevent accidental mutation
 * across threads.
 *
 * \param plugin_names Names of plugins to load.
 * \param gopts Shared options.
 * \return Shared pointer to an immutable map of plugin name -> routine instance.
 */
inline std::shared_ptr<const dRoutinesMap> dynamicRoutinesMap(const std::vector<std::string>&  plugin_names,
                                                              const std::shared_ptr<GOptions>& gopts) {
	auto     log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, GDIGITIZATION_LOGGER);
	GManager manager(gopts);

	auto routines = std::make_shared<dRoutinesMap>();

	for (const auto& plugin : plugin_names) {
		routines->emplace(plugin, manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>(plugin, gopts));
		log->info(0, "dynamicRoutinesMap[", plugin, "]: ", (*routines)[plugin]);
	}

	return routines;
}
} // namespace gdynamicdigitization
