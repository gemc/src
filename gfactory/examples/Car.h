#pragma once

// gemc
#include "gbase.h"

// gfactory
#include "gdl.h"
#include "gfactory_options.h"


class Car : public GBase<Car> {
public:

	~Car() override = default;

	explicit Car(const std::shared_ptr<GOptions>& g) : GBase(g, PLUGIN_LOGGER) {
	}

	virtual void go() = 0;

	void set_loggers([[ maybe_unused ]] const std::shared_ptr<GOptions>&  g) {}

    double generalCarVar = 44;

    // method to dynamically load factories
    // static Car *instantiate(const dlhandle handle) {
    //
    //     if (handle == nullptr) return nullptr;
    //
    //     // must match the extern C declaration in the derived factories
    //     void *maker = dlsym(handle, "CarFactory");
    //
    //     if (maker == nullptr) return nullptr;
    //
    //     typedef Car *(*fptr)();
    //
    //     // static_cast not allowed here
    //     // see http://stackoverflow.com/questions/573294/when-to-use-reinterpret-cast
    //     // need to run the DLL CarFactory function that returns the factory
    //     fptr func = reinterpret_cast<fptr>(reinterpret_cast<void *>(maker));
    //
    //     return func();
    // }

	static Car* instantiate(const dlhandle h, std::shared_ptr<GOptions> g) {
    	if (!h) return nullptr;
    	using fptr = Car* (*)(std::shared_ptr<GOptions>);

    	// Must match the extern "C" declaration in the derived factories.
    	auto sym   = dlsym(h, "CarFactory");
    	if (!sym) return nullptr;

    	auto func = reinterpret_cast<fptr>(sym);
    	return func(g);
    }

};

