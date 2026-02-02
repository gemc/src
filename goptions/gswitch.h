/**
 * @file gswitch.h
 * @brief Definition of \ref GSwitch : the boolean command-line switch type used by \ref GOptions : .
 */

#pragma once

#include <string>

/**
 * @class GSwitch
 * @brief Represents a boolean command-line switch with a description and a status.
 *
 * @details
 * A switch is a presence-based boolean flag:
 * - Default: off (false)
 * - If specified on the command line (e.g. `-gui`), it becomes on (true).
 *
 * Switches are stored by \ref GOptions : and are typically used for enabling/disabling
 * optional behaviors in an application (GUI, interactive mode, logging toggles, etc.).
 */
class GSwitch
{
public:
	/**
	 * @brief Default constructor.
	 *
	 * @details
	 * Constructs an empty \ref GSwitch : with default-initialized members. This constructor is required
	 * for use in associative containers (e.g., `std::map<std::string, GSwitch>`), where default
	 * construction may occur during insertion/lookup.
	 */
	GSwitch() = default;

	/**
	 * @brief Parameterized constructor.
	 *
	 * @details
	 * Initializes the switch description and forces the status to `false` (off).
	 *
	 * @param d Human-readable description of what enabling this switch does.
	 */
	GSwitch(const std::string& d) : description(d), status(false) {
	}

	/**
	 * @brief Turns the switch on.
	 *
	 * @details
	 * Sets the internal status to `true`. In practice, \ref GOptions : calls this when it encounters
	 * the switch name on the command line.
	 */
	void turnOn() { status = true; }

	/**
	 * @brief Turns the switch off.
	 *
	 * @details
	 * Sets the internal status to `false`. This is typically used programmatically (not by parsing)
	 * when a system wants to enforce a default or override state.
	 */
	void turnOff() { status = false; }

	/**
	 * @brief Retrieves the current status of the switch.
	 *
	 * @return `true` if the switch is on; `false` otherwise.
	 */
	bool getStatus() const { return status; }

	/**
	 * @brief Retrieves the description of the switch.
	 *
	 * @return A const reference to the switch's description string.
	 */
	const std::string& getDescription() const { return description; }

private:
	/**
	 * @brief Human-readable description of the switch.
	 *
	 * @details
	 * This is displayed by \ref GOptions::printHelp "printHelp()" and
	 * \ref GOptions::printOptionOrSwitchHelp "printOptionOrSwitchHelp()" .
	 */
	std::string description;

	/**
	 * @brief Current on/off status.
	 *
	 * @details
	 * - `true`  : switch enabled (present on the command line or explicitly enabled)
	 * - `false` : switch disabled (default)
	 */
	bool status{false};
};
