#pragma once

#include "Car.h"

/**
 * @class Tesla
 * @brief Example dynamically-loadable `Car` implementation.
 *
 * This plugin demonstrates:
 * - a derived class with additional state (`autopilot`),
 * - construction via a shared configuration object, and
 * - creation through the exported `CarFactory` symbol.
 */
class Tesla : public Car
{
public:
	/// Inherit the base (const std::shared_ptr<GOptions>&) constructor.
	using Car::Car;

	/**
	 * @brief Construct and initialize the Tesla plugin.
	 *
	 * @param gopts Options/configuration shared with the application.
	 */
	Tesla(const std::shared_ptr<GOptions>& gopts);

	/**
	 * @brief Example behavior for the Tesla plugin.
	 *
	 * Prints internal `autopilot` state and base-class state to demonstrate correctness.
	 */
	void go();

	/**
	 * @brief Example Tesla-specific state.
	 *
	 * Filled by the constructor and printed in \ref Tesla::go "go()".
	 */
	double autopilot[2][2];
};

/**
 * @brief Exported factory function for dynamic loading.
 *
 * This must match the lookup performed by \ref Car::instantiate "Car::instantiate()".
 *
 * @param g Options/configuration to pass into the derived constructor.
 * @return Newly allocated Tesla instance as a `Car*`. Ownership is transferred to the caller.
 */
extern "C" Car* CarFactory(const std::shared_ptr<GOptions>& g) { return static_cast<Car*>(new Tesla(g)); }
