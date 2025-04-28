#ifndef SYSTEMGDMLFACTORY_H
#define SYSTEMGDMLFACTORY_H 1

// gsystem
#include "systemFactory.h"

// system factory
class GSystemGDMLFactory : GSystemFactory {

public:
	// constructor will load the possible location(s) of the geometry and material databases
	GSystemGDMLFactory();

private:

private:
	void loadMaterials(GSystem* system, std::shared_ptr<GLogger> log) override;

	void loadGeometry(GSystem* system, std::shared_ptr<GLogger> log) override;

};


#endif
