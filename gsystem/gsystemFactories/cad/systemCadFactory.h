#ifndef SYSTEMCADFACTORY_H
#define SYSTEMCADFACTORY_H 1

// gsystem
#include "systemFactory.h"

// system factory
class GSystemCADFactory : GSystemFactory {

public:

    // constructor will load the possible location(s) of the geometry and material databases
    GSystemCADFactory();

private:

    virtual void loadMaterials(GSystem *system, int verbosity);

    virtual void loadGeometry(GSystem *system, int verbosity);

};

#endif
