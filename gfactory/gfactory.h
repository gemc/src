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
	explicit GManager(std::shared_ptr<GLogger> logger, std::string name);

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

	/// Create an instance of previously‑registered factory as @c Base*.
	template <class Base>
	[[nodiscard]] Base* CreateObject(std::string_view name) const;

	/// Load a shared library, look up its instantiate symbol, and return object.
	template <class T>
	[[nodiscard]] T* LoadAndRegisterObjectFromLibrary(std::string_view name);

	/// Explicit cleanup (also called by destructor) – idempotent.
	void clearDLMap() noexcept;

private:
	void registerDL(std::string_view name);

	std::unordered_map<std::string, std::unique_ptr<GFactoryBase>> factoryMap_;
	std::unordered_map<std::string, std::unique_ptr<DynamicLib>>   dlMap_;

	std::string              gname_;
	std::shared_ptr<GLogger> log_;
};


inline GManager::GManager(std::shared_ptr<GLogger> logger, std::string name)
	: gname_(std::move(name)), log_(logger) { log_->debug(CONSTRUCTOR,  gname_); }

inline GManager::~GManager() {
	log_->debug(DESTRUCTOR, gname_);
	clearDLMap();
}

inline void GManager::clearDLMap() noexcept { dlMap_.clear(); }

inline void GManager::registerDL(std::string_view name) {
	const std::string filename = std::string{name} + ".gplugin";
	dlMap_.emplace(std::string{name},
	               std::make_unique<DynamicLib>(log_, filename));
	log_->debug(NORMAL, "Loading DL ", name);
}

template <class Derived>
void GManager::RegisterObjectFactory(std::string_view name) {
	factoryMap_.emplace(std::string{name}, std::make_unique<GFactory<Derived>>());
	log_->debug(NORMAL, "Registering ", name, " into factory map");
}

template <class Base>
Base* GManager::CreateObject(std::string_view name) const {
	auto it = factoryMap_.find(std::string{name});
	if (it == factoryMap_.end()) {
		log_->error(ERR_FACTORYNOTFOUND,
		            "Couldn't find factory <", name, "> in factory map.");
	}
	log_->debug(NORMAL, "Creating instance of <", name, "> factory.");
	return static_cast<Base*>(it->second->Create());
}

template <class T>
T* GManager::LoadAndRegisterObjectFromLibrary(std::string_view name) {
	registerDL(name);
	auto& dynamicLib = dlMap_.at(std::string{name});
	if (dynamicLib && dynamicLib->handle) { return T::instantiate(dynamicLib->handle); }
	log_->error(ERR_DLHANDLENOTFOUND, "Plugin ", name, " could not be loaded.");
}
