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
#include "gtranslationTable_options.h"
#include "gdata_options.h"
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

/// Class to manage modifications to a GTouchable using defined modifier weights.
class GTouchableModifiers {
public:
	/**
	 * \brief Constructs GTouchableModifiers with a list of touchable names.
	 *
	 * Initializes internal maps (modifierWeightsMap) for each provided touchable name.
	 *
	 * \param touchableNames A vector of touchable names.
	 */
	explicit GTouchableModifiers(const std::vector<std::string>& touchableNames);

private:
	// Only one of these maps can be filled with values:
	// the size of the map is used by processGTouchableModifiers.

	/// Map holding pairs (id, weight) for each touchable.
	std::map<std::string, std::vector<double>> modifierWeightsMap;

	/// Map holding triplets (id, weight, time) for each touchable.
	std::map<std::string, std::vector<double>> modifierWeightsAndTimesMap;

public:
	/**
	 * \brief Inserts a new (id, weight) pair for given touchable.
	 * \param touchableName The name of the touchable.
	 * \param idValue The identifier value.
	 * \param weight The weight.
	 */
	void insertIdAndWeight(const std::string& touchableName, int idValue, double weight);

	/**
	 * \brief Inserts a new (id, weight, time) triplet for given touchable.
	 * \param touchableName The name of the touchable.
	 * \param idValue The identifier value.
	 * \param weight The weight.
	 * \param time The time.
	 */
	void insertIdWeightAndTime(const std::string& touchableName, int idValue, double weight, double time);

	/**
	 * \brief Normalizes the modifier weights using a total weight.
	 *
	 * For the weights-only map, each weight is divided by totalWeight.
	 * For the weights-and-times map, the weight (second element of each triplet) is normalized.
	 *
	 * \param touchableName The name of the touchable.
	 * \param totalWeight The total weight used for normalization.
	 */
	void assignOverallWeight(const std::string& touchableName, double totalWeight);

	/**
	 * \brief Checks whether only weight modifiers (without time) are defined.
	 *
	 * \return True if modifierWeightsMap is not empty.
	 */
	[[nodiscard]] inline bool isWeightsOnly() const { return !modifierWeightsMap.empty(); }

	/**
	 * \brief Gets the modifier weights vector for a given touchable.
	 * \param touchableName The touchable name.
	 * \return The vector of weights.
	 *
	 * \note This function will crash if the key is not declared.
	 */
	inline std::vector<double> getModifierWeightsVector(const std::string& touchableName) { return modifierWeightsMap[touchableName]; }

	/**
	 * \brief Gets the modifier weights and time vector for a given touchable.
	 * \param touchableName The touchable name.
	 * \return The vector of weights and times.
	 *
	 * \note This function will crash if the key is not declared.
	 */
	inline std::vector<double> getModifierWeightsAndTimeVector(const std::string& touchableName) { return modifierWeightsAndTimesMap[touchableName]; }
};

/// Abstract base class for dynamic digitization functionality.
class GDynamicDigitization {
public:
	/**
	 * \brief Virtual destructor.
	 */
	virtual ~GDynamicDigitization() = default;

	/**
	 * \brief Processes the step time.
	 *
	 * Returns the global time from the post-step point of the current G4Step.
	 *
	 * \param gTouchID Pointer to the GTouchable.
	 * \param thisStep Pointer to the current G4Step.
	 * \return The global time.
	 */
	[[nodiscard]] double processStepTime(const std::shared_ptr<GTouchable>& gTouchID, [[maybe_unused]] G4Step* thisStep) {
		check_if_log_defined();
		digi_logger->debug(NORMAL, "GDynamicDigitization::process step time");
		return processStepTimeImpl(gTouchID, thisStep);
	}

	[[nodiscard]] virtual double processStepTimeImpl(const std::shared_ptr<GTouchable>& gTouchID, [[maybe_unused]] G4Step* thisStep);


	/**
	 * \brief Processes a GTouchable based on a G4Step.
	 *
	 * If the computed time cell index matches the current one (or if it was unset),
	 * the original touchable is returned; otherwise, a new touchable is created.
	 *
	 * \param gTouchID Pointer to the original GTouchable.
	 * \param thisStep Pointer to the current G4Step.
	 * \return A vector of GTouchable pointers.
	 */
	[[nodiscard]] std::vector<std::shared_ptr<GTouchable>> processTouchable(std::shared_ptr<GTouchable> gtouchable, G4Step* thisStep) {
		check_if_log_defined();
		digi_logger->debug(NORMAL, "GDynamicDigitization::process gtouchable");
		return processTouchableImpl(std::move(gtouchable), thisStep);
	}

	[[nodiscard]] virtual std::vector<std::shared_ptr<GTouchable>> processTouchableImpl(std::shared_ptr<GTouchable> gtouchable, G4Step* thisStep);

	/**
	 * \brief Processes touchable modifiers.
	 *
	 * This function applies modifiers from a GTouchableModifiers object.
	 *
	 * \param gTouchID Pointer to the original GTouchable.
	 * \param gmods A GTouchableModifiers object.
	 * \return A vector of modified GTouchable pointers.
	 */
	[[nodiscard]] std::vector<std::shared_ptr<GTouchable>> processGTouchableModifiers(const std::shared_ptr<GTouchable>& gTouchID, GTouchableModifiers gmods) {
		check_if_log_defined();
		digi_logger->debug(NORMAL, "GDynamicDigitization::process gtouchable modifiers");
		return processGTouchableModifiersImpl(gTouchID, std::move(gmods));
	}

	virtual std::vector<std::shared_ptr<GTouchable>> processGTouchableModifiersImpl([[maybe_unused]] const std::shared_ptr<GTouchable>& gTouchID,
	                                                                                [[maybe_unused]] const GTouchableModifiers&         gmods);

	/**
	 * \brief Collects true hit information into a GTrueInfoData object.
	 *
	 * Integrates all information built in GHit::addHitInfosForBitset.
	 *
	 * \param ghit Pointer to a GHit.
	 * \param hitn Hit index.
	 * \return A pointer to a newly created GTrueInfoData object.
	 */
	[[nodiscard]] std::unique_ptr<GTrueInfoData> collectTrueInformation(GHit* ghit, size_t hitn) {
		check_if_log_defined();
		digi_logger->info(2, "GDynamicDigitization::collect true information for hit number ", hitn, " with size ", ghit->nsteps(), " steps");
		return collectTrueInformationImpl(ghit, hitn);
	}

	[[nodiscard]] virtual std::unique_ptr<GTrueInfoData> collectTrueInformationImpl(GHit* ghit, size_t hitn);

	/**
	 * \brief Digitizes hit information into a GDigitizedData object.
	 *
	 * \param ghit Pointer to a GHit.
	 * \param hitn Hit index.
	 * \return A pointer to a GDigitizedData object, or nullptr if not implemented.
	 */
	[[nodiscard]] std::unique_ptr<GDigitizedData> digitizeHit(GHit* ghit, [[maybe_unused]] size_t hitn) {
		check_if_log_defined();
		digi_logger->info(2, "GDynamicDigitization::digitize  hit number ", hitn, " with size ", ghit->nsteps(), " steps");
		return digitizeHitImpl(ghit, hitn);
	}

	[[nodiscard]] virtual std::unique_ptr<GDigitizedData> digitizeHitImpl([[maybe_unused]] GHit* ghit, [[maybe_unused]] size_t hitn) { return nullptr; }

	/**
	 * \brief Loads digitization constants.
	 *
	 * \param runno Run number.
	 * \param variation Variation string.
	 * \return True if successful, false otherwise.
	 */
	[[nodiscard]] bool loadConstants([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) {
		check_if_log_defined();
		digi_logger->debug(NORMAL, "GDynamicDigitization::load constants");
		return loadConstantsImpl(runno, variation);
	}

	virtual bool loadConstantsImpl([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) { return false; }

	/**
	 * \brief Loads the translation table.
	 *
	 * \param runno Run number.
	 * \param variation Variation string.
	 * \return True if successful, false otherwise.
	 */
	[[nodiscard]] bool loadTT([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) {
		check_if_log_defined();
		digi_logger->debug(NORMAL, "GDynamicDigitization::load Translation Table for run ", runno, " with variation ", variation);
		return loadTTImpl(runno, variation);
	}

	virtual bool loadTTImpl([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) { return false; }

	/**
	 * \brief Sets hardware-level charge and time information in the digitized data.
	 *
	 * Sets the gdata variable TIMEATELECTRONICS (used by RunAction to identify eventFrameIndex)
	 * and includes the translation table (hardware address: crate/slot/channel) in the digitized data.
	 * Exits with an error if the translation table is not defined.
	 *
	 * \param time Time value (ns).
	 * \param q Charge value.
	 * \param ghit Pointer to the GHit.
	 * \param gdata Pointer to the GDigitizedData - caller keeps ownership.
	 */
	void chargeAndTimeAtHardware(int time, int q, const GHit* ghit, GDigitizedData& gdata);

	/**
	 * \brief Pure virtual function to initialize readout specifications.
	 *
	 * Must be implemented by derived classes.
	 *
	 * \return True if initialization is successful, false otherwise.
	 */
	[[nodiscard]] bool defineReadoutSpecs() {
		check_if_log_defined();
		digi_logger->debug(NORMAL, "GDynamicDigitization::define readout specs");
		return defineReadoutSpecsImpl();
	};
	virtual bool defineReadoutSpecsImpl() = 0;

	/// After init, we never mutate these:
	std::shared_ptr<const GReadoutSpecs>     readoutSpecs;
	std::shared_ptr<const GTranslationTable> translationTable;

	/**
	 * \brief Dynamically instantiates a GDynamicDigitization object from a dynamic library.
	 *
	 * \param handle Handle to the dynamic library.
	 * \return A pointer to a GDynamicDigitization instance, or nullptr if instantiation fails.
	 */
	static GDynamicDigitization* instantiate(const dlhandle handle) {
		if (handle == nullptr) return nullptr;
		// Must match the extern "C" declaration in the derived factories.
		void* maker = dlsym(handle, "GDynamicDigitizationFactory");
		if (maker == nullptr) return nullptr;
		typedef GDynamicDigitization*(*fptr)();
		// Use reinterpret_cast as required.
		fptr func = reinterpret_cast<fptr>(reinterpret_cast<void*>(maker));
		return func();
	}

	// decides if the hit should be processed or not
	virtual bool decisionToSkipHit(double energy) {
		if (energy == 0 && !recordZeroEdep) { return true; }

		return false;
	}


	/**
	 * \brief Sets the loggers for the digitization process.
	 *
	 * Initializes data_logger, tt_logger, and digi_logger based on the provided GOptions.
	 *
	 * \param g Pointer to GOptions.
	 */
	void set_loggers(GOptions* const g) {
		gopts       = g;
		data_logger = std::make_shared<GLogger>(gopts.value(), DATA_LOGGER, "data");
		tt_logger   = std::make_shared<GLogger>(gopts.value(), TRANSLATIONTABLE_LOGGER, "translation table");
		digi_logger = std::make_shared<GLogger>(gopts.value(), GDIGITIZATION_LOGGER, "digitization");
	}

private:
	bool recordZeroEdep = false;

protected:
	/// Optional pointer to GOptions.
	std::optional<GOptions*> gopts;
	/// Data, Translation Tables, and digitization loggers.
	std::shared_ptr<GLogger> data_logger, tt_logger, digi_logger;

	/**
	 * \brief Checks that all required loggers and options are defined.
	 *
	 * If any required logger is missing, prints an error message and exits.
	 */
	void check_if_log_defined() const {
		if (!gopts.has_value() || !gopts.value() || data_logger == nullptr || tt_logger == nullptr || digi_logger ==
		    nullptr) {
			std::cerr << KRED <<
				"Fatal Error: GDynamicDigitization: goption is not set for this plugin or one of the loggers is null."
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

inline std::shared_ptr<GDynamicDigitization> load_dynamicRoutine(const std::string& plugin_name, GOptions* gopts) {
	auto     log = std::make_shared<GLogger>(gopts, GDIGITIZATION_LOGGER, "loader");
	GManager manager(log);
	return manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>(plugin_name, gopts);
}

//  the returned map is shared and immutable
inline std::shared_ptr<const dRoutinesMap> dynamicRoutinesMap(const std::vector<std::string>& plugin_names, GOptions* gopts) {
	auto     log = std::make_shared<GLogger>(gopts, GDIGITIZATION_LOGGER, "loader");
	GManager manager(log);

	auto routines = std::make_shared<dRoutinesMap>();

	for (const auto& plugin : plugin_names) {
		routines->emplace(plugin, manager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>(plugin, gopts));
		log->info(0, "dynamicRoutinesMap[", plugin, "]: ", (*routines)[plugin]);
	}

	return routines; // shared_ptr<const ...> prevents mutation
}


} // namespace gstreamer
