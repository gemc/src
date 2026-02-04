#pragma once

#include <iostream>

/**
 * \class GModifier
 * \brief Describes a post-load modification to a single volume.
 *
 * A GModifier is a small value-object that carries *optional* adjustments that
 * will be applied to a target volume after it is loaded by a GSystemFactory and
 * inserted into a GSystem.
 *
 * Typical uses include:
 * - applying an additional translation ("shift") to a volume placement;
 * - applying an additional rotation ("tilts") to a volume placement;
 * - toggling the existence flag (remove a volume from the world).
 *
 * \note This class is intentionally minimal: it stores the modifier values and
 * provides trivial getters. It does not apply the modifications itself.
 */
class GModifier
{
public:
	/**
	 * \brief Construct a modifier record.
	 *
	 * \param n The target volume name. This must match the key used to find the
	 *          volume within the world (typically the volume name as stored in the geometry).
	 * \param s The shift expression to apply (e.g. `"0*cm, 0*cm, 1*cm"`), or
	 *          GSYSTEMNOMODIFIER when not used.
	 * \param t The tilt expression to apply (e.g. `"0*deg, 0*deg, -10*deg"`), or
	 *          GSYSTEMNOMODIFIER when not used.
	 * \param existence Existence toggle: \c true means the volume is present,
	 *                  \c false means it should be removed/disabled.
	 */
	GModifier(const std::string& n, const std::string& s, const std::string& t, bool existence) :
		name(n),
		shift(s),
		tilts(t),
		isPresent(existence) {
	}

	/// \brief Default copy constructor (value-object semantics).
	GModifier(const GModifier& other) = default;

private:
	std::string name;      ///< Target volume name (used as lookup key during modifier application).
	std::string shift;     ///< Additional translation to apply to the volume placement.
	std::string tilts;     ///< Additional rotation to apply to the volume placement.
	bool        isPresent; ///< Existence flag: \c true means keep volume, \c false disables/removes it.

	/**
	 * \brief Stream operator for logging/debug printing.
	 *
	 * Prints a compact representation of the modifier so that callers can
	 * include it in logger output.
	 */
	friend std::ostream& operator<<(std::ostream& stream, const GModifier& gm) {
		stream << "GModifier: " << gm.name << " shift: " << gm.shift << " tilts: " << gm.tilts << " isPresent: "
			<< gm.isPresent;
		return stream;
	}

public:
	/// \brief Returns the modifier target volume name.
	std::string getName() { return name; }

	/// \brief Returns the configured shift expression.
	std::string getShift() { return shift; }

	/// \brief Returns the configured tilt expression.
	std::string getTilts() { return tilts; }

	/// \brief Returns whether the target volume should exist in the final world.
	[[nodiscard]] bool getExistence() const { return isPresent; }
};
