#include "ginternalDigitization.h"

/**
 * \brief Defines readout specifications for flux digitization.
 *
 * This method sets the electronic readout time-window, the grid start time,
 * and the hit bitset (which defines what hit information is stored). It also
 * calls check_if_log_defined() to ensure that all required loggers are set.
 *
 * \return True if the readout specifications were successfully defined.
 */
bool GFluxDigitization::defineReadoutSpecsImpl() {
	// Ensure all loggers and options are properly defined.
	check_if_log_defined();

	// Define the electronic readout time-window for the detector.
	float timeWindow = 10;                  // electronic readout time-window of the detector
	// Define the start time for the time grid.
	float gridStartTime = 0;                // defines the windows grid
	// Define the hit bitset: "000001" indicates which information to store in the hit.
	HitBitSet hitBitSet("000001");

	// Create a new GReadoutSpecs object using the provided parameters.
	// The digi_logger (obtained via set_loggers()) is used for logging.
	readoutSpecs = new GReadoutSpecs(timeWindow, gridStartTime, hitBitSet, digi_logger);

	return true;
}

/**
 * \brief Digitizes a hit and returns a GDigitizedData object.
 *
 * This method extracts digitized information from the provided GHit.
 * It retrieves the first identifier from the hit, then creates a GDigitizedData
 * object and populates it with several variables (hit index, total energy deposited,
 * average time, particle ID, and total energy).
 * The check_if_log_defined() call ensures that necessary loggers are available.
 *
 * \param ghit Pointer to the GHit to digitize.
 * \param hitn The hit index (used to label the hit).
 * \return A pointer to the newly created GDigitizedData object.
 */
GDigitizedData* GFluxDigitization::digitizeHitImpl(GHit *ghit, size_t hitn) {
	// Ensure all required loggers and options are set.
	check_if_log_defined();

	// Retrieve the first identifier from the hit's identity vector.
	GIdentifier identity = ghit->getGID().front();

	// Create a new GDigitizedData object using the hit and the data_logger.
	auto gdata = new GDigitizedData(ghit, data_logger);

	// Include digitized variables into the GDigitizedData object:
	// 1. Use the identifier's name and value.
	gdata->includeVariable(identity.getName(), identity.getValue());
	// 2. Include the hit number.
	gdata->includeVariable("hitn", static_cast<int>(hitn));
	// 3. Include the total energy deposited.
	gdata->includeVariable("totEdep", ghit->getTotalEnergyDeposited());
	// 4. Include the average time of the hit.
	gdata->includeVariable("time", ghit->getAverageTime());
	// 5. Include the particle ID.
	gdata->includeVariable("pid", ghit->getPid());
	// 6. Include the total energy (from the hit's energy vector).
	gdata->includeVariable("totalE", ghit->getE());

	return gdata;
}
