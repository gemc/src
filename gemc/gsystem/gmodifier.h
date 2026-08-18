#pragma once

#include <iostream>
#include <optional>
#include <string>
#include <utility>

/**
 * \ingroup gemc_gsystem_modifiers
 *
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
	 * \param s The optional shift expression to apply (e.g. `"0*cm, 0*cm, 1*cm"`).
	 * \param t The optional tilt expression to apply (e.g. `"0*deg, 0*deg, -10*deg"`).
	 * \param existence Existence toggle: \c true means the volume is present,
	 *                  \c false means it should be removed/disabled.
	 */
	GModifier(std::string n, std::optional<std::string> s, std::optional<std::string> t, bool existence) :
		name(std::move(n)),
		shift(std::move(s)),
		tilts(std::move(t)),
		isPresent(existence) {
	}

	/// \brief Default copy constructor (value-object semantics).
	GModifier(const GModifier& other) = default;

private:
	std::string                name;  ///< Target volume name (used as lookup key during modifier application).
	std::optional<std::string> shift; ///< Additional translation to apply to the volume placement.
	std::optional<std::string> tilts; ///< Additional rotation to apply to the volume placement.
	bool isPresent; ///< Existence flag: \c true means keep volume, \c false disables/removes it.

	/**
	 * \brief Stream operator for logging/debug printing.
	 *
	 * Prints a compact representation of the modifier so that callers can
	 * include it in logger output.
	 */
	friend std::ostream& operator<<(std::ostream& stream, const GModifier& gm) {
		stream << "GModifier: " << gm.name << " shift: " << gm.shift.value_or("none")
			<< " tilts: " << gm.tilts.value_or("none") << " isPresent: " << gm.isPresent;
		return stream;
	}

public:
	/// \brief Returns the modifier target volume name.
	[[nodiscard]] const std::string& getName() const { return name; }

	/// \brief Returns the configured shift expression.
	[[nodiscard]] const std::optional<std::string>& getShift() const { return shift; }

	/// \brief Returns the configured tilt expression.
	[[nodiscard]] const std::optional<std::string>& getTilts() const { return tilts; }

	/// \brief Returns whether the target volume should exist in the final world.
	[[nodiscard]] bool getExistence() const { return isPresent; }
};
