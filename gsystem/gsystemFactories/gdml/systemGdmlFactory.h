#ifndef SYSTEMGDMLFACTORY_H
#define SYSTEMGDMLFACTORY_H 1

// gsystem
#include "systemFactory.h"

// system factory
class GSystemGDMLFactory : GSystemFactory
{

public:

	// constructor will load the possible location(s) of the geometry and material databases
	GSystemGDMLFactory();


private:

	virtual void loadMaterials(GSystem *system, int verbosity);
	virtual void loadGeometry(GSystem *system, int verbosity);

};


#endif
