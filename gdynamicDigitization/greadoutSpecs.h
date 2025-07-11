#pragma once

/**
 * \file GReadoutSpecs.h
 * \brief Defines the GReadoutSpecs class for readout electronics specifications.
 *
 * \mainpage GReadoutSpecs Module
 *
 * \section intro_sec Introduction
 * The GReadoutSpecs class encapsulates properties of the readout electronics, such as
 * the electronic readout time-window, grid start time, and the hit bitset that defines
 * what information is stored in a hit. These specifications are used by both GTouchable
 * and GHit to determine the appropriate time cell index for a hit.
 *
 * \section usage_sec Usage
 * To use this class, a plugin should construct a GReadoutSpecs object by providing:
 *  - The electronic readout time-window.
 *  - The grid start time.
 *  - A HitBitSet defining what hit information to store.
 *  - A logger for informational output.
 *
 * Once constructed, the hit bitset can be retrieved via getHitBitSet(), and the time cell
 * index can be computed for a given time using timeCellIndex().
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */

#include "ghitConventions.h" // For HitBitSet
#include "glogger.h"

#include <cmath>  // Use <cmath> for std::floor

/**
 * \class GReadoutSpecs
 * \brief Specifies the readout electronics parameters.
 *
 * This class holds parameters such as the electronic readout time-window,
 * grid start time, and the hit bitset. These values are used to determine which
 * information is stored in a hit and to compute the corresponding time cell index.
 */
class GReadoutSpecs {

private:
	double    timeWindow;    ///< Electronic readout time-window of the detector.
	double    gridStartTime; ///< The start time of the time grid.
	HitBitSet hitBitSet;     ///< Defines which hit information to be stored.

public:
	/**
	 * \brief Constructs a GReadoutSpecs object.
	 *
	 * This constructor is intended to be called by plugins to set readout specifications.
	 *
	 * \param tw The electronic readout time-window.
	 * \param gst The grid start time.
	 * \param hbs The hit bitset defining what information to store.
	 * \param log Pointer to a GLogger instance for logging.
	 */
	GReadoutSpecs(const double tw, const double gst, const HitBitSet hbs, const std::shared_ptr<GLogger>& log) :
		timeWindow(tw),
		gridStartTime(gst),
		hitBitSet(hbs) {
		log->info(1, "GReadoutSpecs: timeWindow=", timeWindow, ", gridStartTime=", gridStartTime, ", hitBitSet=",
		          hitBitSet);
	}

public:
	/**
	 * \brief Retrieves the hit bitset.
	 *
	 * \return The HitBitSet defining the stored hit information.
	 */
	[[nodiscard]] inline HitBitSet getHitBitSet() const { return hitBitSet; }

	/**
	 * \brief Computes the time cell index for a given time.
	 *
	 * For readout detectors, this function calculates the cell index within the
	 * electronic time window. The formula used is:
	 * \f[
	 * \text{cell index} = \left\lfloor \frac{\text{time} - \text{gridStartTime}}{\text{timeWindow}} \right\rfloor + 1
	 * \f]
	 *
	 * \param time The time value (e.g., global time from the simulation).
	 * \return The computed time cell index as an integer.
	 */
	[[nodiscard]] inline int timeCellIndex(double time) const { return static_cast<int>(std::floor((time - gridStartTime) / timeWindow) + 1); }
};


#include <type_traits>

// At compile time, make sure GReadoutSpecs has trivial destructors,
static_assert(std::is_trivially_destructible_v<GReadoutSpecs>);

