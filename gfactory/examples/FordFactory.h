#pragma once

#include "Car.h"

/**
 * @class Ford
 * @brief Example dynamically-loadable `Car` implementation.
 *
 * This class lives in a plugin library and is instantiated through the exported
 * `extern "C"` symbol `CarFactory`.
 */
class Ford : public Car
{
public:
	/// Inherit the base (const std::shared_ptr<GOptions>&) constructor.
	using Car::Car;

	/**
	 * @brief Construct and initialize example state.
	 *
	 * @param gopts Options/configuration shared with the application.
	 */
	Ford(const std::shared_ptr<GOptions>& gopts);

	/**
	 * @brief Example behavior for the Ford plugin.
	 *
	 * The example prints out internal state to demonstrate the call path.
	 */
	void go();

	/**
	 * @brief Example plugin-specific state.
	 *
	 * Initialized by the constructor and printed by \ref Ford::go "go()".
	 */
	double fordVar[2]{};
};

/**
 * @brief Exported factory function for dynamic loading.
 *
 * The base class \ref Car::instantiate "Car::instantiate()" resolves this symbol via \c dlsym and calls it.
 *
 * @param g Options/configuration to pass into the derived constructor.
 * @return Newly allocated derived object as a base pointer. Ownership is transferred to the caller.
 */
extern "C" Car* CarFactory(const std::shared_ptr<GOptions>& g) {
	return static_cast<Car*>(new Ford(g));
}
