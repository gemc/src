#pragma once

#include "gdl.h"

class Car {
public:
    virtual void go() = 0;

    virtual ~Car() = default;

    float generalCarVar = 44;

    // method to dynamically load factories
    static Car *instantiate(const dlhandle handle) {

        if (handle == nullptr) return nullptr;

        // must match the extern C declaration in the derived factories
        void *maker = dlsym(handle, "CarFactory");

        if (maker == nullptr) return nullptr;

        typedef Car *(*fptr)();

        // static_cast not allowed here
        // see http://stackoverflow.com/questions/573294/when-to-use-reinterpret-cast
        // need to run the DLL CarFactory function that returns the factory
        fptr func = reinterpret_cast<fptr>(reinterpret_cast<void *>(maker));

        return func();
    }
};

