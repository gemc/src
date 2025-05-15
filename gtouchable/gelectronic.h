#pragma once

#include <ostream>
#include <vector>

/**
 * @brief Represents an electronic module address.
 *
 * This class encapsulates the crate, slot, and channel details used to identify a specific electronic module.
 * Comparison is based on the mode, which defines the granularity of the comparison:
 * 	 mode == 0: crate only
 * 	 mode == 1: crate, slot
 * 	 mode == 2: crate, slot, channel
 * For example, if mode = 1:
 * 3,3,1 is the same as 3,3,4
 * In mode 2 they would be different
 */
struct GElectronic {

public:

	/**
	* @brief Constructs a GElectronic object with given address and mode.
	*
	* @param c Crate number.
	* @param s Slot number.
	* @param ch Channel number.
	* @param m Mode for comparison (0: crate, 1: crate and slot, 2: crate, slot, and channel).
	*/
	GElectronic(int c, int s, int ch, int m);

	/**
	* @brief Default constructor.
	*
	* Initializes GElectronic with uninitialized values.
	* empty constructor needed because of the map declaration in the translation table
	*/
	GElectronic();

	/**
	* @brief Sets the hardware address.
	*
	* @param c Crate number.
	* @param s Slot number.
	* @param ch Channel number.
	*/
	void setHAddress(int c, int s, int ch);

	/**
	* @brief Gets the hardware address.
	*
	* @return A vector containing {crate, slot, channel}.
	*/
	std::vector<int> getHAddress();

private:
	int crate;    ///< Crate number.
	int slot;     ///< Slot number.
	int channel;  ///< Channel number.
	int mode;     ///< Comparison mode.


	/**
	* @brief Equality operator based on mode.
	*
	* Mode defines the granularity of the comparison:
	* - 0: Compare only crate.
	* - 1: Compare crate and slot.
	* - 2: Compare crate, slot, and channel.
	*
	* @param ge The GElectronic object to compare.
	* @return True if the objects are considered equal based on the mode.
	*/
	bool operator==(const GElectronic& ge) const;

	/// Overloaded output operator for GElectronic.
	friend std::ostream &operator<<(std::ostream &stream, const GElectronic &ge);


};

