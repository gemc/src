#ifndef GDYNAMICDIGITIZATION_H
#define GDYNAMICDIGITIZATION_H 1



#include "greadoutSpecs.h"
#include "gfactory_options.h"

// gemc
#include "gdl.h"
#include "gtouchable.h"
#include "ghit.h"
#include "gDigitizedData.h"
#include "gTrueInfoData.h"
#include "gtranslationTable.h"
#include "gfactory_options.h"
#include "gtranslationTable_options.h"
#include "gdata_options.h"
#include "gdynamicdigitization_options.h"

// c++
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
	explicit GTouchableModifiers(std::vector<std::string> touchableNames);

private:
	// Only one of these maps can be filled with values:
	// the size of the map is used by processGTouchableModifiers.

	/// Map holding pairs (id, weight) for each touchable.
	std::map<std::string, std::vector<double>> modifierWeightsMap;

	/// Map holding triplets (id, weight, time) for each touchable.
	std::map<std::string, std::vector<double>> modifierWeightsAndTimesMap;

public:
	/**
	 * \brief Inserts a new (id, weight) pair for a given touchable.
	 * \param touchableName The name of the touchable.
	 * \param idValue The identifier value.
	 * \param weight The weight.
	 */
	void insertIdAndWeight(std::string touchableName, int idValue, double weight);

	/**
	 * \brief Inserts a new (id, weight, time) triplet for a given touchable.
	 * \param touchableName The name of the touchable.
	 * \param idValue The identifier value.
	 * \param weight The weight.
	 * \param time The time.
	 */
	void insertIdWeightAndTime(std::string touchableName, int idValue, double weight, double time);

	/**
	 * \brief Normalizes the modifier weights using a total weight.
	 *
	 * For the weights-only map, each weight is divided by totalWeight.
	 * For the weights-and-times map, the weight (second element of each triplet) is normalized.
	 *
	 * \param touchableName The name of the touchable.
	 * \param totalWeight The total weight used for normalization.
	 */
	void assignOverallWeight(std::string touchableName, double totalWeight);

	/**
	 * \brief Checks whether only weight modifiers (without time) are defined.
	 *
	 * \return True if modifierWeightsMap is not empty.
	 */
	inline bool isWeightsOnly() {
		return !modifierWeightsMap.empty();
	}

	/**
	 * \brief Gets the modifier weights vector for a given touchable.
	 * \param touchableName The touchable name.
	 * \return The vector of weights.
	 *
	 * \note This function will crash if the key is not declared.
	 */
	inline std::vector<double> getModifierWeightsVector(const std::string &touchableName) {
		return modifierWeightsMap[touchableName];
	}

	/**
	 * \brief Gets the modifier weights and time vector for a given touchable.
	 * \param touchableName The touchable name.
	 * \return The vector of weights and times.
	 *
	 * \note This function will crash if the key is not declared.
	 */
	inline std::vector<double> getModifierWeightsAndTimeVector(const std::string &touchableName) {
		return modifierWeightsAndTimesMap[touchableName];
	}
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
	float processStepTime(GTouchable *gTouchID, G4Step *thisStep) {
		check_if_log_defined();
		digi_logger.value()->debug(NORMAL, "GDynamicDigitization::process step time");
		return processStepTimeImpl(gTouchID, thisStep);

	}
	virtual float processStepTimeImpl(GTouchable *gTouchID, G4Step *thisStep);


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
	std::vector<GTouchable *> processTouchable(GTouchable *gTouchID, G4Step *thisStep) {
		check_if_log_defined();
		digi_logger.value()->debug(NORMAL, "GDynamicDigitization::process gtouchable");
		return processTouchableImpl(gTouchID, thisStep);
	}
	virtual std::vector<GTouchable *> processTouchableImpl(GTouchable *gTouchID, G4Step *thisStep);

	/**
	 * \brief Processes touchable modifiers.
	 *
	 * This function applies modifiers from a GTouchableModifiers object.
	 *
	 * \param gTouchID Pointer to the original GTouchable.
	 * \param gmods A GTouchableModifiers object.
	 * \return A vector of modified GTouchable pointers.
	 */
	std::vector<GTouchable *> processGTouchableModifiers(GTouchable *gTouchID, GTouchableModifiers gmods) {
		check_if_log_defined();
		digi_logger.value()->debug(NORMAL, "GDynamicDigitization::process gtouchable modifiers");
		return processGTouchableModifiersImpl(gTouchID, gmods);
	}
	virtual std::vector<GTouchable *> processGTouchableModifiersImpl(GTouchable *gTouchID, GTouchableModifiers gmods);

	/**
	 * \brief Collects true hit information into a GTrueInfoData object.
	 *
	 * Integrates all information built in GHit::addHitInfosForBitset.
	 *
	 * \param ghit Pointer to a GHit.
	 * \param hitn Hit index.
	 * \return A pointer to a newly created GTrueInfoData object.
	 */
	GTrueInfoData *collectTrueInformation(GHit *ghit, size_t hitn) {
		check_if_log_defined();
		digi_logger.value()->debug(NORMAL, "GDynamicDigitization::collect true information");
		return collectTrueInformationImpl(ghit, hitn);
	}
	virtual GTrueInfoData *collectTrueInformationImpl(GHit *ghit, size_t hitn);

	/**
	 * \brief Digitizes hit information into a GDigitizedData object.
	 *
	 * \param ghit Pointer to a GHit.
	 * \param hitn Hit index.
	 * \return A pointer to a GDigitizedData object, or nullptr if not implemented.
	 */
	GDigitizedData *digitizeHit([[maybe_unused]] GHit *ghit, [[maybe_unused]] size_t hitn) {
		check_if_log_defined();
		digi_logger.value()->debug(NORMAL, "GDynamicDigitization::digitize hit");
		return digitizeHitImpl(ghit, hitn);
	}
	virtual GDigitizedData *digitizeHitImpl([[maybe_unused]] GHit *ghit, [[maybe_unused]] size_t hitn) { return nullptr; }

	/**
	 * \brief Loads digitization constants.
	 *
	 * \param runno Run number.
	 * \param variation Variation string.
	 * \return True if successful, false otherwise.
	 */
	bool loadConstants([[maybe_unused]] int runno, [[maybe_unused]] std::string const &variation) {
		check_if_log_defined();
		digi_logger.value()->debug(NORMAL, "GDynamicDigitization::load constants");
		return loadConstantsImpl(runno, variation);
	}
	virtual bool loadConstantsImpl([[maybe_unused]] int runno, [[maybe_unused]] std::string const &variation) { return false; }

	/**
	 * \brief Loads the translation table.
	 *
	 * \param runno Run number.
	 * \param variation Variation string.
	 * \return True if successful, false otherwise.
	 */
	bool loadTT([[maybe_unused]] int runno, [[maybe_unused]] std::string const &variation) {
		check_if_log_defined();
		digi_logger.value()->debug(NORMAL, "GDynamicDigitization::load Translation Table");
		return loadTTImpl(runno, variation);
	}
	virtual bool loadTTImpl([[maybe_unused]] int runno, [[maybe_unused]] std::string const &variation) { return false; }

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
	 * \param gdata Pointer to the GDigitizedData.
	 */
	void chargeAndTimeAtHardware(int time, int q, GHit *ghit, GDigitizedData *gdata);

	/**
	 * \brief Pure virtual function to initialize readout specifications.
	 *
	 * Must be implemented by derived classes.
	 *
	 * \return True if initialization is successful, false otherwise.
	 */
	bool defineReadoutSpecs() {
		check_if_log_defined();
		digi_logger.value()->debug(NORMAL, "GDynamicDigitization::define readout specs");
		return defineReadoutSpecsImpl();
	};
	virtual bool defineReadoutSpecsImpl() = 0;

	/// Pointer to the readout specifications.
	GReadoutSpecs *readoutSpecs = nullptr;
	/// Pointer to the translation table.
	GTranslationTable *translationTable = nullptr;

	/**
	 * \brief Dynamically instantiates a GDynamicDigitization object from a dynamic library.
	 *
	 * \param handle Handle to the dynamic library.
	 * \return A pointer to a GDynamicDigitization instance, or nullptr if instantiation fails.
	 */
	static GDynamicDigitization *instantiate(const dlhandle handle) {
		if (handle == nullptr) return nullptr;
		// Must match the extern "C" declaration in the derived factories.
		void *maker = dlsym(handle, "GDynamicDigitizationFactory");
		if (maker == nullptr) return nullptr;
		typedef GDynamicDigitization *(*fptr)();
		// Use reinterpret_cast as required.
		fptr func = reinterpret_cast<fptr>(reinterpret_cast<void *>(maker));
		return func();
	}

	/**
	 * \brief Sets the loggers for the digitization process.
	 *
	 * Initializes data_logger, tt_logger, and digi_logger based on the provided GOptions.
	 *
	 * \param g Pointer to GOptions.
	 */
	void set_loggers(GOptions *const g) {
		gopts = g;
		data_logger = new GLogger(gopts.value(), DATA_LOGGER);
		tt_logger = new GLogger(gopts.value(), TRANSLATIONTABLE_LOGGER);
		digi_logger = new GLogger(gopts.value(), GDIGITIZATION_LOGGER);
	}

protected:
	/// Optional pointer to GOptions.
	std::optional<GOptions *> gopts;
	/// Optional pointer to the data logger.
	std::optional<GLogger *> data_logger;
	/// Optional pointer to the translation table logger.
	std::optional<GLogger *> tt_logger;
	/// Optional pointer to the digitization logger.
	std::optional<GLogger *> digi_logger;

	/**
	 * \brief Checks that all required loggers and options are defined.
	 *
	 * If any required logger is missing, prints an error message and exits.
	 */
	void check_if_log_defined() {
		if (!gopts.has_value() || !data_logger.has_value() || !tt_logger.has_value() || !digi_logger.has_value()) {
			std::cerr << KRED << "Fatal Error: GDynamicDigitization: goption is not set for this plugin." << std::endl;
			std::cerr << "The set_loggers function needs to be called." << std::endl;
			std::cerr << "For example: dynamicRoutines[\"ctof\"]->set_loggers(gopts);" << std::endl;
			std::cerr << RST << std::endl;
			exit(1);
		}
	}
};

#endif
