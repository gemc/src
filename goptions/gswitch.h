#pragma once

#include <string>

/**
 * @class GSwitch
 * @brief Represents a switch with a description and a status.
 *
 * The `GSwitch` class encapsulates a switch's description and its status,
 * allowing the switch to be turned on or off, and providing methods to
 * query its current state.
 */
class GSwitch {

public:

    /**
     * @brief Default constructor.
     *
     * Constructs an empty `GSwitch` object. This constructor is necessary
     * for the `GSwitch` to be used in containers like `unordered_map<std::string, GSwitch>`.
     */
    GSwitch() = default;

    /**
     * @brief Parameterized constructor.
     *
     * Initializes the switch with a description and sets its status to `false`.
     *
     * @param d The description of the switch.
     */
    GSwitch(const std::string &d) : description(d), status(false) {}

    /**
     * @brief Turns the switch on.
     *
     * Sets the switch's status to `true`, indicating that the switch is on.
     */
    void turnOn() { status = true; }

    /**
     * @brief Turns the switch off.
     *
     * Sets the switch's status to `false`, indicating that the switch is off.
     */
    void turnOff() { status = false; }

    /**
     * @brief Retrieves the current status of the switch.
     *
     * @return `true` if the switch is on, `false` otherwise.
     */
    bool getStatus() const { return status; }

    /**
     * @brief Retrieves the description of the switch.
     *
     * @return A `const` reference to the switch's description.
     */
    const std::string &getDescription() const { return description; }

private:
    /**
     * @brief The description of the switch.
     */
    std::string description;

    /**
     * @brief The status of the switch.
     *
     * Represents whether the switch is on (`true`) or off (`false`).
     * Initialized to `false`.
     */
    bool status{false};

};

