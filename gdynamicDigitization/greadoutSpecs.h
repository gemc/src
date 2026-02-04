#pragma once

/**
 * \file greadoutSpecs.h
 * \brief Readout electronics specifications used by time-binning and hit organization.
 *
 * GReadoutSpecs represents the (simple) electronics timing model needed by digitization:
 * a time window defines the width of one electronics time bin, and a grid start time
 * defines the phase (offset) of that binning. A HitBitSet encodes which hit information
 * is computed/stored.
 *
 * \note
 * This header intentionally does not declare a \c \\mainpage. Module-level documentation
 * for gdynamic digitization lives in gdynamicdigitizationDoxy.h.
 */

#include "ghitConventions.h" // HitBitSet
#include "glogger.h"

#include <cmath>
#include <type_traits>

/**
 * \class GReadoutSpecs
 * \brief Immutable readout timing and storage specification.
 *
 * Instances of this class are typically constructed by digitization plugins inside
 * \ref GDynamicDigitization::defineReadoutSpecs "defineReadoutSpecs()" and then shared
 * as an immutable object for subsequent processing.
 *
 * \note
 * The class is designed to be trivially destructible (enforced by a static_assert below).
 */
class GReadoutSpecs
{
private:
	/// Width of a single electronics time cell (time unit follows project conventions; commonly ns).
	double timeWindow;

	/// Time offset (origin) of the electronics time grid.
	double gridStartTime;

	/// Bitset controlling which hit information is computed/stored.
	HitBitSet hitBitSet;

public:
	/**
	 * \brief Constructs a GReadoutSpecs object.
	 *
	 * The constructor logs the parameters at verbosity level 1.
	 *
	 * \param tw Electronics time window (width of one time cell).
	 * \param gst Grid start time (time offset for binning).
	 * \param hbs Hit bitset controlling which hit fields are stored/computed.
	 * \param log Logger used for informational output.
	 */
	GReadoutSpecs(const double tw, const double gst, const HitBitSet hbs, const std::shared_ptr<GLogger>& log) :
		timeWindow(tw),
		gridStartTime(gst),
		hitBitSet(hbs) {
		log->info(1, "GReadoutSpecs: timeWindow=", timeWindow, ", gridStartTime=", gridStartTime, ", hitBitSet=",
		          hitBitSet);
	}

	/**
	 * \brief Returns the configured hit bitset.
	 *
	 * \return The HitBitSet that defines what hit information is stored.
	 */
	[[nodiscard]] inline HitBitSet getHitBitSet() const { return hitBitSet; }

	/**
	 * \brief Computes the 1-based electronics time-cell index for a given time.
	 *
	 * The current convention returns a 1-based index:
	 * \f[
	 * \mathrm{index} = \left\lfloor \frac{t - \mathrm{gridStartTime}}{\mathrm{timeWindow}} \right\rfloor + 1
	 * \f]
	 *
	 * This is commonly used to:
	 * - split GTouchables when a hit spans multiple electronics time bins
	 * - label digitized hits by an electronics "frame" index
	 *
	 * \param time Time value to bin (time unit follows project conventions; commonly ns).
	 * \return 1-based time-cell index as an integer.
	 */
	[[nodiscard]] inline int timeCellIndex(double time) const {
		return static_cast<int>(std::floor((time - gridStartTime) / timeWindow) + 1);
	}
};

// At compile time, make sure GReadoutSpecs has trivial destructors.
static_assert(std::is_trivially_destructible_v<GReadoutSpecs>);
