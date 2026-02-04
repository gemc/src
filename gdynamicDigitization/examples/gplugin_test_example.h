#pragma once

/**
 * \file gplugin_test_example.h
 * \brief Example dynamic digitization plugin used by the gdynamic digitization examples.
 *
 * This example plugin is intentionally small and demonstrates the required plugin entry points:
 * - \ref GDynamicDigitization::defineReadoutSpecs "defineReadoutSpecs()"
 * - \ref GDynamicDigitization::loadConstants "loadConstants()"
 * - \ref GDynamicDigitization::loadTT "loadTT()"
 * - \ref GDynamicDigitization::digitizeHit "digitizeHit()"
 *
 * \note
 * This header intentionally contains the authoritative Doxygen documentation for the example API.
 * The corresponding .cc file contains implementation-only (non-Doxygen) comments to avoid
 * duplicated \c \\param sections when Doxygen merges declarations and definitions.
 */

// gdynamic
#include "../gdynamicdigitization.h"

// c++
#include <string>

/**
 * \class GPlugin_test_example
 * \brief Minimal test plugin for dynamic digitization.
 *
 * This class is designed for:
 * - demonstrating the plugin surface expected by gdynamic digitization
 * - serving as a small integration test for the dynamic loader
 *
 * It stores a few example configuration variables (scalar/array/vector/string) and
 * creates a small translation table in memory.
 */
class GPlugin_test_example : public GDynamicDigitization
{
public:
	/// Inherit the base constructor (const std::shared_ptr<GOptions>&).
	using GDynamicDigitization::GDynamicDigitization;

	/**
	 * \brief Defines readout specifications for this example plugin.
	 *
	 * This is the minimal required hook that every plugin must implement.
	 * The routine demonstrates assigning:
	 * - a fixed electronics time window
	 * - a grid start time (time-grid origin)
	 * - a hit bitset controlling which hit fields are computed/stored
	 *
	 * \return true when readout specifications are assigned.
	 */
	bool defineReadoutSpecsImpl() override;

	/**
	 * \brief Loads example constants used by this plugin.
	 *
	 * This routine demonstrates how a plugin can initialize internal configuration
	 * state (members) that later influences digitization.
	 *
	 * It sets:
	 * - var1 : scalar example value
	 * - var2 : small fixed-size array example value
	 * - var3 : vector example value
	 * - var4 : string example value
	 *
	 * \param runno Run number (used for logging in this example).
	 * \param variation Variation string (used for logging in this example).
	 * \return true to indicate success.
	 */
	bool loadConstantsImpl(int runno, std::string const& variation) override;

	/**
	 * \brief Builds a minimal in-memory translation table for the example.
	 *
	 * This demonstrates how a plugin can map detector identities (vectors of ints)
	 * to electronics addresses.
	 *
	 * \param runno Unused in this example.
	 * \param variation Unused in this example.
	 * \return true to indicate success.
	 */
	bool loadTTImpl([[maybe_unused]] int runno, [[maybe_unused]] std::string const& variation) override;

	/**
	 * \brief Digitizes the provided hit into a small example output record.
	 *
	 * This example demonstrates:
	 * - creating a GDigitizedData record
	 * - computing a "voltage" from total deposited energy
	 * - building a synthetic "digi_time" by scaling and summing hit step times
	 *
	 * \param ghit Input hit to digitize. Ownership stays with the caller.
	 * \param hitn Hit index (unused in this example).
	 * \return Newly allocated digitized record.
	 */
	[[nodiscard]] std::unique_ptr<GDigitizedData> digitizeHitImpl(GHit* ghit, [[maybe_unused]] size_t hitn) override;

private:
	/// Example scalar configuration value.
	double var1 = 1;

	/// Example fixed-size array configuration value.
	int var2[2] = {0, 0};

	/// Example variable-length configuration value.
	std::vector<double> var3;

	/// Example string configuration value.
	std::string var4;

	/// Translation table created by loadTTImpl().
	std::shared_ptr<GTranslationTable> translationTable;
};
