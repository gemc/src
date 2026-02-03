#pragma once
/**
 * @file gfactory.h
 * @brief Generic factory/manager for GEMC plugin objects.
 *
 * This header provides a small, type-erased factory system that supports:
 * - **Static factories**: register a C++ type and instantiate it by name.
 * - **Dynamic factories**: load a shared library and instantiate an object via a well-known symbol.
 *
 * The core entry point is \ref GManager "GManager".
 *
 * @ingroup Factory
 */

// C++
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

// gfactory
#include "gdl.h"
#include "gbase.h"
#include "gfactory_options.h"

/**
 * @class GFactoryBase
 * @brief Type-erased factory interface used by \ref GManager "GManager".
 *
 * \ref GManager "GManager" stores factories for many unrelated types inside a single container.
 * This is achieved by type-erasing the creation operation into \ref GFactoryBase::Create "Create()",
 * which returns a raw pointer as `void*`.
 *
 * @note The returned pointer is owned by the caller of \ref GManager::CreateObject "CreateObject()".
 * The caller is responsible for deleting it (typically via a base-class pointer).
 */
class GFactoryBase
{
public:
	/// Virtual destructor for safe deletion through base pointer.
	virtual ~GFactoryBase() = default;

	/**
	 * @brief Instantiate the concrete product.
	 *
	 * @return A raw, heap-allocated object pointer cast to `void*`.
	 *
	 * @warning Ownership is transferred to the caller.
	 */
	[[nodiscard]] virtual void* Create() = 0;
};

/**
 * @class GFactory
 * @brief Concrete factory that creates objects of type @p T.
 *
 * This factory assumes that @p T can be constructed from `const std::shared_ptr<GOptions>&`.
 * The options object is captured at factory construction time and forwarded during instantiation.
 *
 * @tparam T Concrete type to allocate with `new T(...)`.
 *
 * @note This class exists primarily to satisfy the \ref GFactoryBase "GFactoryBase" interface.
 */
template <class T>
class GFactory final : public GFactoryBase
{
public:
	/**
	 * @brief Construct a factory bound to a specific configuration/options instance.
	 *
	 * @param gopts Shared options/configuration object forwarded to each constructed instance.
	 *
	 * @note The factory stores a copy of the shared pointer; it does not take exclusive ownership.
	 */
	explicit GFactory(const std::shared_ptr<GOptions>& gopts)
		: gopts_(gopts) {
		static_assert(std::is_constructible_v<T, const std::shared_ptr<GOptions>&>,
		              "T must be constructible from const std::shared_ptr<GOptions>&");
	}

	/**
	 * @brief Allocate a new instance of @p T.
	 * @return The newly allocated object as `void*` (caller owns it).
	 */
	[[nodiscard]] void* Create() override { return static_cast<void*>(new T(gopts_)); }

private:
	/// Options used when constructing each instance.
	std::shared_ptr<GOptions> gopts_;
};

/**
 * @class GManager
 * @brief Factory registry and dynamic-library manager for run-time creation of plugin objects.
 *
 * \ref GManager "GManager" provides two related services:
 *
 * 1. **Static factory registry**
 *    - Register a concrete type under a string key (e.g., `"triangle"`).
 *    - Instantiate it later using \ref GManager::CreateObject "CreateObject()".
 *
 * 2. **Dynamic library loading**
 *    - Load a module (`<name>.gplugin`) into the process.
 *    - Instantiate an object by calling a well-known entry point (via `dlsym` inside the product type).
 *
 * ### Ownership and lifetime
 * - For static creation (\ref GManager::CreateObject "CreateObject()"), the returned raw pointer is
 *   owned by the caller.
 * - For dynamic creation (\ref GManager::LoadAndRegisterObjectFromLibrary "LoadAndRegisterObjectFromLibrary()"),
 *   the returned `std::shared_ptr<T>` ensures:
 *   - the object is deleted when the shared pointer is released, and
 *   - the dynamic library remains loaded at least as long as the object is alive
 *     (the deleter captures the `std::shared_ptr<DynamicLib>`).
 *
 * ### Error handling
 * If a factory key is unknown or a library cannot be loaded, the manager logs an error using its logger.
 * The exact policy after logging depends on the logger configuration (e.g., may abort, throw, or continue).
 *
 * @ingroup Factory
 */
class GManager : public GBase<GManager>
{
public:
	/**
	 * @brief Construct a manager instance.
	 *
	 * @param gopt Shared options object used to configure the base logger and behavior.
	 *
	 * @note The manager uses the `PLUGIN_LOGGER` channel for plugin-related output.
	 */
	explicit GManager(const std::shared_ptr<GOptions>& gopt) : GBase(gopt, PLUGIN_LOGGER) {
	}

	/// No copy – the manager owns unique resources (factory objects and loaded libraries).
	GManager(const GManager&)            = delete;
	GManager& operator=(const GManager&) = delete;

	/// Allow move for container support.
	GManager(GManager&&) noexcept            = default;
	GManager& operator=(GManager&&) noexcept = default;

	/**
	 * @brief Destructor.
	 *
	 * Calls \ref GManager::clearDLMap "clearDLMap()" to release any loaded libraries.
	 */
	~GManager() override { clearDLMap(); }

	/**
	 * @brief Register a concrete factory under a string key.
	 *
	 * @tparam Derived Concrete type to instantiate.
	 * @param name Factory key used later by \ref GManager::CreateObject "CreateObject()".
	 *
	 * @note This overload registers the type only. The factory must still be constructible as required by
	 *       \ref GFactory "GFactory".
	 */
	template <class Derived>
	void RegisterObjectFactory(std::string_view name);

	/**
	 * @brief Register a concrete factory under a string key using a specific options object.
	 *
	 * @tparam Derived Concrete type to instantiate.
	 * @param name  Factory key used later by \ref GManager::CreateObject "CreateObject()".
	 * @param gopts Options forwarded to the factory and then to constructed objects.
	 */
	template <class Derived>
	void RegisterObjectFactory(std::string_view name, const std::shared_ptr<GOptions>& gopts);

	/**
	 * @brief Create an instance of a previously registered factory.
	 *
	 * @tparam Base Base type to cast the created object to.
	 * @param name  Factory key.
	 *
	 * @return A pointer to a heap-allocated object cast to @p Base.
	 *
	 * @warning The caller owns the returned pointer and must delete it.
	 */
	template <class Base>
	[[nodiscard]] Base* CreateObject(std::string_view name) const;

	/**
	 * @brief Load a plugin library and instantiate an object from it.
	 *
	 * The product type @p T must provide a static method `instantiate(dlhandle, std::shared_ptr<GOptions>)`
	 * that performs the symbol lookup and returns a raw pointer.
	 *
	 * @tparam T Product base type.
	 * @param name  Plugin name (used to form `<name>.gplugin`).
	 * @param gopts Options to pass to the instantiated object.
	 *
	 * @return `std::shared_ptr<T>` owning the created object; its deleter also retains the library.
	 */
	template <class T>
	[[nodiscard]] std::shared_ptr<T> LoadAndRegisterObjectFromLibrary(std::string_view                 name,
	                                                                  const std::shared_ptr<GOptions>& gopts);

	/**
	 * @brief Release all loaded dynamic libraries.
	 *
	 * This is safe to call multiple times.
	 */
	void clearDLMap() noexcept;

private:
	/**
	 * @brief Register/load a dynamic library into the internal map.
	 *
	 * @param name Plugin base name; the library filename is constructed as `<name>.gplugin`.
	 */
	void registerDL(std::string_view name);

	/// Map from factory key to type-erased factory.
	std::unordered_map<std::string, std::unique_ptr<GFactoryBase>> factoryMap_;

	/// Map from plugin key to the loaded library handle.
	std::unordered_map<std::string, std::shared_ptr<DynamicLib>> dlMap_;

	/// Optional human-readable manager name (currently unused here).
	std::string gname;
};


inline void GManager::clearDLMap() noexcept {
	// Clearing the map releases std::shared_ptr<DynamicLib> instances.
	// Each DynamicLib destructor closes its dlopen handle.
	dlMap_.clear();
}

inline void GManager::registerDL(std::string_view name) {
	// Convention: plugins are packaged as "<name>.gplugin".
	const std::string filename = std::string{name} + ".gplugin";

	// Store the DynamicLib in a shared_ptr so it can be safely captured by
	// a shared_ptr deleter in LoadAndRegisterObjectFromLibrary().
	dlMap_.emplace(std::string{name},
	               std::make_shared<DynamicLib>(log, filename));
	log->debug(NORMAL, "Loading DL ", name);
}

template <class Derived>
void GManager::RegisterObjectFactory(std::string_view name) {
	// Note: the factory is stored type-erased (GFactoryBase) but remains responsible for creating Derived.
	factoryMap_.emplace(std::string{name}, std::make_unique<GFactory<Derived>>());
	log->debug(NORMAL, "Registering ", name, " into factory map");
}

template <class Derived>
void GManager::RegisterObjectFactory(std::string_view name, const std::shared_ptr<GOptions>& gopts) {
	factoryMap_.emplace(std::string{name}, std::make_unique<GFactory<Derived>>(gopts));
	log->debug(NORMAL, "Registering ", name, " into factory map");
}

template <class Base>
Base* GManager::CreateObject(std::string_view name) const {
	auto it = factoryMap_.find(std::string{name});
	if (it == factoryMap_.end()) {
		log->error(ERR_FACTORYNOTFOUND,
		           "Couldn't find factory <", name, "> in factory map.");
	}
	log->debug(NORMAL, "Creating instance of <", name, "> factory.");

	// Type-erased creation returns void*; caller requests a Base* view.
	return static_cast<Base*>(it->second->Create());
}

template <class T>
std::shared_ptr<T> GManager::LoadAndRegisterObjectFromLibrary(std::string_view                 name,
                                                              const std::shared_ptr<GOptions>& gopts) {
	registerDL(name);
	auto pluginName = std::string{name};
	auto pluginLib  = dlMap_.at(pluginName); // shared_ptr<DynamicLib>

	if (pluginLib && pluginLib->handle) {
		// The product type performs the symbol lookup and raw allocation.
		T* raw = T::instantiate(pluginLib->handle, gopts);

		// Standardize logger wiring on the instance.
		raw->set_loggers(gopts);

		// Return shared_ptr<T> with a deleter that captures pluginLib so the library stays loaded
		// until the object is destroyed.
		return std::shared_ptr<T>(raw, [pluginLib](T* ptr) {
			delete ptr;
			// pluginLib keeps the dlopen handle alive until ptr is destroyed.
		});
	}

	log->error(ERR_DLHANDLENOTFOUND, "Plugin ", name, " could not be loaded.");
}
