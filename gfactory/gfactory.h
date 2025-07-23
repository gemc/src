#pragma once
/**
 * @file gfactory.h
 * @brief Generic factory/manager for GEMC plugin objects.
 *
 * @ingroup Factory
 */

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

#include "gdl.h"
#include "glogger.h"


/**
 * @class GFactoryBase
 * @brief Abstract creator used by GManager through type‑erased pointers.
 */
class GFactoryBase {
public:
	virtual ~GFactoryBase() = default;
	/// Pure virtual instantiation hook implemented by the templated concrete factory.
	[[nodiscard]] virtual void* Create() = 0;
};

/**
 * @brief Concrete factory that creates objects of type @p T.
 * @tparam T The derived type to instantiate via <code>new T()</code>.
 */
template <class T>
class GFactory final : public GFactoryBase {
public:
	[[nodiscard]] void* Create() override { return new T(); }
};

/**
 * @class GManager
 * @brief Owns factories and dynamically‑loaded libraries, providing run‑time creation.
 *
 * Usage pattern:
 * ```
 * gemc::GManager mgr(log, "ShapeManager");
 * mgr.RegisterObjectFactory<Circle>("circle");
 * auto* c = mgr.CreateObject<Shape>("circle");
 * delete c;
 * ```
 *
 * @ingroup Factory
 */
class GManager {
public:
	/// Construct with logger and a human‑readable name.
	explicit GManager(std::shared_ptr<GLogger> logger, std::string description = "");

	/// No copy – the manager owns unique resources.
	GManager(const GManager&)            = delete;
	GManager& operator=(const GManager&) = delete;
	/// Allow move for container support.
	GManager(GManager&&) noexcept            = default;
	GManager& operator=(GManager&&) noexcept = default;

	~GManager();

	/// Register a concrete factory under @p name.
	template <class Derived>
	void RegisterObjectFactory(std::string_view name);

	/// Create an instance of previously registered factory as @c Base*.
	template <class Base>
	[[nodiscard]] Base* CreateObject(std::string_view name) const;

	/// Load a shared library, look up its instantiate symbol, and return object.
	template <class T>
	[[nodiscard]] T* LoadAndRegisterObjectFromLibrary(std::string_view name, GOptions* gopts);

	/// Explicit cleanup (also called by destructor) – idempotent.
	void clearDLMap() noexcept;

	std::shared_ptr<DynamicLib> getDLHandle(std::string_view name) const {
		auto it = dlMap_.find(std::string{name});
		if (it != dlMap_.end()) return it->second;
		return nullptr;
	}

private:
	void registerDL(std::string_view name);

	std::unordered_map<std::string, std::unique_ptr<GFactoryBase>> factoryMap_;
	std::unordered_map<std::string, std::shared_ptr<DynamicLib>>   dlMap_;

	std::string              gname;
	std::shared_ptr<GLogger> log;
};


inline GManager::GManager(std::shared_ptr<GLogger> logger, std::string description)
	: log(logger) {
	gname = log->get_verbosity_name() + description;
	log->debug(CONSTRUCTOR, gname);
}

inline GManager::~GManager() {
	log->debug(DESTRUCTOR, gname);
	clearDLMap();
}

inline void GManager::clearDLMap() noexcept { dlMap_.clear(); }

inline void GManager::registerDL(std::string_view name) {
	const std::string filename = std::string{name} + ".gplugin";
	dlMap_.emplace(std::string{name},
	               std::make_shared<DynamicLib>(log, filename));
	log->debug(NORMAL, "Loading DL ", name);
}

template <class Derived>
void GManager::RegisterObjectFactory(std::string_view name) {
	factoryMap_.emplace(std::string{name}, std::make_unique<GFactory<Derived>>());
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
	return static_cast<Base*>(it->second->Create());
}

template <class T>
T* GManager::LoadAndRegisterObjectFromLibrary(std::string_view name, GOptions* gopts) {
	registerDL(name);
	auto& dynamicLib = dlMap_.at(std::string{name});
	if (dynamicLib && dynamicLib->handle) {
		auto factory = T::instantiate(dynamicLib->handle);
		// in c++20 it is possible to use introspection with concept thuse removing the
		// need of having to define a set_loggers function
		// it didn't work on 5/21/2025 so for now the function is required to be defined
		factory->set_loggers(gopts);
		return factory;
	}
	log->error(ERR_DLHANDLENOTFOUND, "Plugin ", name, " could not be loaded.");
}
