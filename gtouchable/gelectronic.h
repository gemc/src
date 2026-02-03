#pragma once

#include <ostream>
#include <vector>

/**
 * @file gelectronic.h
 * @brief Defines GElectronic, a compact hardware address used by digitization and translation tables.
 */

/**
 * @brief Represents an electronic module address (crate/slot/channel) with configurable comparison granularity.
 *
 * A GElectronic instance encapsulates:
 * - crate number
 * - slot number
 * - channel number
 * - comparison mode (granularity)
 *
 * The comparison mode defines which parts of the address are considered significant when comparing two addresses:
 * - mode == 0: compare crate only
 * - mode == 1: compare crate and slot
 * - mode == 2: compare crate, slot, and channel
 *
 * Example (mode == 1):
 * - (3, 3, 1) is considered the same as (3, 3, 4) because the channel is ignored.
 * In mode == 2, those two would be different.
 *
 * Typical usage:
 * - Translation tables and lookup maps can choose the comparison granularity they need (crate-only vs full address).
 */
struct GElectronic
{
public:
	/**
	 * @brief Constructs a GElectronic with a specific hardware address and comparison mode.
	 *
	 * @param c Crate number.
	 * @param s Slot number.
	 * @param ch Channel number.
	 * @param m Comparison mode:
	 *   - 0: crate only
	 *   - 1: crate and slot
	 *   - 2: crate, slot, and channel
	 */
	GElectronic(int c, int s, int ch, int m);

	/**
	 * @brief Default constructor.
	 *
	 * Initializes the address and mode to the "uninitialized" sentinel used by the framework.
	 * This constructor exists because some containers (e.g. maps used by translation tables) require
	 * default-constructible value types.
	 */
	GElectronic();

	/**
	 * @brief Sets the hardware address fields (crate/slot/channel).
	 *
	 * The comparison mode is not changed by this call.
	 *
	 * @param c Crate number.
	 * @param s Slot number.
	 * @param ch Channel number.
	 */
	void setHAddress(int c, int s, int ch);

	/**
	 * @brief Returns the hardware address as a vector of three integers.
	 *
	 * @return A vector containing {crate, slot, channel} in that order.
	 */
	std::vector<int> getHAddress();

private:
	int crate;   ///< Crate number.
	int slot;    ///< Slot number.
	int channel; ///< Channel number.
	int mode;    ///< Comparison mode (granularity), see class documentation.

	/**
	 * @brief Equality operator using the configured comparison mode.
	 *
	 * This comparison is intentionally mode-dependent to support lookup keys of varying granularity.
	 *
	 * @param ge The GElectronic object to compare with.
	 * @return True if the objects are considered equal under the current mode.
	 */
	bool operator==(const GElectronic& ge) const;

	/**
	 * @brief Stream output helper for diagnostics and logging.
	 *
	 * Prints the address fields in a human-readable form.
	 */
	friend std::ostream& operator<<(std::ostream& stream, const GElectronic& ge);
};
