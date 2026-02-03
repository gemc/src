#pragma once

// gemc
#include "gbase.h"

// gfactory
#include "gdl.h"
#include "gfactory_options.h"

/**
 * @class Car
 * @brief Example base class used to demonstrate dynamic factory loading.
 *
 * This type is intentionally small and serves as a reference pattern for:
 * - a plugin base class deriving from `GBase`, and
 * - a static \ref Car::instantiate "instantiate()" helper that resolves an extern "C" factory symbol
 *   from a \c dlopen handle and returns a newly allocated instance.
 *
 * @note In real modules, the base class typically defines the stable ABI/API that plugins implement.
 */
class Car : public GBase<Car>
{
public:
	/// Virtual destructor for polymorphic deletion.
	~Car() override = default;

	/**
	 * @brief Construct the base with a shared options/configuration instance.
	 *
	 * @param g Shared options object used to initialize logging via `GBase`.
	 */
	explicit Car(const std::shared_ptr<GOptions>& g) : GBase(g, PLUGIN_LOGGER) {
	}

	/**
	 * @brief Example pure-virtual behavior implemented by each plugin.
	 *
	 * In the example program, this is invoked through a `std::shared_ptr<Car>`
	 * returned by the dynamic loader.
	 */
	virtual void go() = 0;

	/**
	 * @brief Wire loggers into the object.
	 *
	 * This is called by \ref GManager::LoadAndRegisterObjectFromLibrary "LoadAndRegisterObjectFromLibrary()"
	 * after instantiation. For the example, it is a no-op.
	 */
	void set_loggers([[ maybe_unused ]] const std::shared_ptr<GOptions>& g) {
	}

	/**
	 * @brief Example data member shared by all derived cars.
	 *
	 * Used by the example to show that base-class state is accessible through the loaded object.
	 */
	double generalCarVar = 44;

	// Historical note:
	// The block below shows a previous approach that returned a no-arg factory function. The current example
	// passes GOptions into the factory so the created object can be configured consistently.

	/**
	 * @brief Resolve the plugin factory symbol and instantiate a derived `Car`.
	 *
	 * This helper performs the dynamic lookup of the extern "C" factory function from the shared library.
	 * The derived plugin library must export a symbol with the exact name:
	 * - `CarFactory`
	 *
	 * The expected signature is:
	 * - `extern "C" Car* CarFactory(const std::shared_ptr<GOptions>&);`
	 *
	 * @param h POSIX dynamic-library handle (from \c dlopen).
	 * @param g Options/configuration to pass into the derived constructor.
	 *
	 * @return A raw pointer to a newly allocated derived `Car`, or null on failure.
	 *
	 * @warning Ownership of the returned pointer is with the caller. In this module, the pointer is
	 *          immediately wrapped in `std::shared_ptr<Car>` by the manager.
	 */
	static Car* instantiate(const dlhandle h, std::shared_ptr<GOptions> g) {
		if (!h) return nullptr;

		// Function pointer type matching the exported extern "C" symbol.
		using fptr = Car* (*)(std::shared_ptr<GOptions>);

		// Must match the extern "C" declaration in the derived factories.
		// Symbol resolution uses the POSIX API \c dlsym.
		auto sym = dlsym(h, "CarFactory");
		if (!sym) return nullptr;

		// The cast is required because dlsym returns void*.
		// This pattern is common in plugin loaders: resolve symbol -> cast -> call.
		auto func = reinterpret_cast<fptr>(sym);
		return func(g);
	}
};
