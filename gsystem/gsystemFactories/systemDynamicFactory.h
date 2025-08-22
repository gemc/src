#pragma once

// gemc
#include "goptions.h"
#include "gbase.h"

// system plugin factory
// this file is kept here in case in the future we'll need a gsystem plugin

class GSystemDynamicFactory: public GBase<GSystemDynamicFactory> {
public:
	virtual void loadSystem(GSystem* s, int verbosity) = 0;

	virtual ~GSystemDynamicFactory() = default;

	// this method must be present for the dynamic loaded factories
	// static GSystemDynamicFactory* instantiate(const dlhandle handle) {
	// 	if (handle == nullptr) return nullptr;
	//
	// 	// must match the extern C declaration in the derived factories
	// 	void* maker = dlsym(handle, "GSystemDynamicFactory");
	//
	// 	if (maker == nullptr) return nullptr;
	//
	// 	typedef GSystemDynamicFactory*(*fptr)();
	//
	// 	// static_cast not allowed here
	// 	// see http://stackoverflow.com/questions/573294/when-to-use-reinterpret-cast
	// 	// need to run the DLL GSystemDynamicFactory function that returns the factory
	// 	fptr func = reinterpret_cast<fptr>(reinterpret_cast<void*>(maker));
	//
	// 	return func();
	// }

	static GSystemDynamicFactory* instantiate(const dlhandle h, std::shared_ptr<GOptions> g) {
		if (!h) return nullptr;
		using fptr = GSystemDynamicFactory* (*)(std::shared_ptr<GOptions>);

		// Must match the extern "C" declaration in the derived factories.
		auto sym   = dlsym(h, "GSystemDynamicFactory");
		if (!sym) return nullptr;

		auto func = reinterpret_cast<fptr>(sym);
		return func(g);
	}

};
