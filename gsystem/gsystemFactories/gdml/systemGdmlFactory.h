#pragma once

// gsystem
#include "systemFactory.h"

/**
 * \class GSystemGDMLFactory
 * \brief Placeholder factory for GDML-based systems.
 *
 * Current status:
 * - \ref GSystemGDMLFactory::loadGeometry "loadGeometry()" is a stub.
 * - \ref GSystemGDMLFactory::loadMaterials "loadMaterials()" is a stub.
 *
 * Intended future behavior:
 * - Parse a GDML file and create corresponding volumes/material mappings,
 *   or import a geometry and then synthesize GVolume records similarly to CAD import.
 */
class GSystemGDMLFactory : public GSystemFactory
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GSystemFactory::GSystemFactory;

private:
	void loadMaterials(GSystem* system) override;
	void loadGeometry(GSystem* system) override;
};
