#pragma once

// gsystem
#include "systemFactory.h"

// system factory
class GSystemGDMLFactory : GSystemFactory {

public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GSystemFactory::GSystemFactory;

private:

private:
	void loadMaterials(GSystem* system) override;

	void loadGeometry(GSystem* system) override;

};

