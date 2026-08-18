#pragma once

#include <ostream>
#include <stdexcept>
#include <vector>

/**
 * \file gelectronic.h
 * \brief Defines GElectronic, a compact hardware address used by digitization and translation tables.
 */

/**
 * \brief Represents an electronic module address (crate/slot/channel) with configurable comparison granularity.
 *
 * A GElectronic instance encapsulates:
 * - crate number
 * - slot number
 * - channel number
 * - comparison mode (granularity)
 *
 * The comparison mode defines which parts of the address are considered significant when comparing two
 * addresses:
 * - \c crate: compare crate only
 * - \c crate_slot: compare crate and slot
 * - \c crate_slot_channel: compare crate, slot, and channel
 *
 * Example (mode == \c crate_slot):
 * - (3, 3, 1) is considered the same as (3, 3, 4) because the channel is ignored.
 * In mode == \c crate_slot_channel, those two would be different.
 *
 * Typical usage:
 * - Translation tables and lookup maps can choose the comparison granularity they need (crate-only vs full
 *   address).
 */
struct GElectronic
{
public:
	/** \brief Selects which hardware-address fields participate in comparison. */
	enum class ComparisonMode {
		crate,
		crate_slot,
		crate_slot_channel
	};

	/**
	 * \brief Constructs a GElectronic with a specific hardware address and comparison mode.
	 *
	 * \param c Crate number.
	 * \param s Slot number.
	 * \param ch Channel number.
	 * \param comparison_mode Hardware-address comparison granularity.
	 */
	GElectronic(int c, int s, int ch, ComparisonMode comparison_mode);

	/**
	 * \brief Sets the hardware address fields (crate/slot/channel).
	 *
	 * The comparison mode is not changed by this call.
	 *
	 * \param c Crate number.
	 * \param s Slot number.
	 * \param ch Channel number.
	 */
	void setHAddress(int c, int s, int ch);

	/**
	 * \brief Returns the hardware address as a vector of three integers.
	 *
	 * \return A vector containing {crate, slot, channel} in that order.
	 */
	[[nodiscard]] std::vector<int> getHAddress() const;

private:
	static void validateHAddress(int crate, int slot, int channel);

	int crate;   ///< Crate number.
	int slot;    ///< Slot number.
	int channel; ///< Channel number.
	ComparisonMode mode; ///< Comparison granularity, see class documentation.

	/**
	 * \brief Equality operator using the configured comparison mode.
	 *
	 * This comparison is intentionally mode-dependent to support lookup keys of varying granularity.
	 *
	 * \param ge The GElectronic object to compare with.
	 * \return True if the objects are considered equal under the current mode.
	 */
	bool operator==(const GElectronic& ge) const;

	/**
	 * \brief Stream output helper for diagnostics and logging.
	 *
	 * Prints the address fields in a human-readable form.
	 */
	friend std::ostream& operator<<(std::ostream& stream, const GElectronic& ge);
};
