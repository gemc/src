#pragma once

// gemc
#include "goptions.h"
#include "gbase.h"

// system plugin factory
// this file is kept here in case in the future we'll need a gsystem plugin

/**
 * \class GSystemDynamicFactory
 * \brief Optional plugin interface for dynamically-loaded system factories.
 *
 * This interface is currently retained as a future extension point: it defines the
 * minimal API contract that a dynamically-loaded factory would expose.
 *
 * Plugin contract:
 * - A shared library exports an extern "C" symbol named \c "GSystemDynamicFactory".
 * - That symbol is a function returning a \c GSystemDynamicFactory* and taking
 *   \c std::shared_ptr<GOptions> as argument.
 *
 * The helper \ref GSystemDynamicFactory::instantiate "instantiate()" locates the symbol
 * using \c dlsym and invokes it.
 *
 * \note This header intentionally uses \c for external dynamic-link concepts.
 */
class GSystemDynamicFactory : public GBase<GSystemDynamicFactory>
{
public:
	/**
	 * \brief Load a system with an explicit verbosity.
	 *
	 * \param s Target system to populate.
	 * \param verbosity Verbosity level (factory-specific interpretation).
	 */
	virtual void loadSystem(GSystem* s, int verbosity) = 0;

	virtual ~GSystemDynamicFactory() = default;

	/**
	 * \brief Instantiate a dynamic factory from a shared-library handle.
	 *
	 * \param h Dynamic library handle (dlopen-style).
	 * \param g Shared options/configuration instance.
	 * \return Factory instance pointer, or \c nullptr if the symbol is missing.
	 *
	 * \details The symbol must be named exactly \c "GSystemDynamicFactory" and
	 * must match the expected signature.
	 */
	static GSystemDynamicFactory* instantiate(const dlhandle h, std::shared_ptr<GOptions> g) {
		if (!h) return nullptr;
		using fptr = GSystemDynamicFactory* (*)(std::shared_ptr<GOptions>);

		// Must match the extern "C" declaration in the derived factories.
		auto sym = dlsym(h, "GSystemDynamicFactory");
		if (!sym) return nullptr;

		auto func = reinterpret_cast<fptr>(sym);
		return func(g);
	}
};
