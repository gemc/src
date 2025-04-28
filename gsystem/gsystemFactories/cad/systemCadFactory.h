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
	void loadMaterials(GSystem* system, std::shared_ptr<GLogger> log) override;

	void loadGeometry(GSystem* system, std::shared_ptr<GLogger> log) override;

};

#endif
