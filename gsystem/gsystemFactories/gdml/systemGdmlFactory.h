#pragma once

// gsystem
#include "systemFactory.h"

/**
 * \ingroup gemc_gsystem_factories_gdml
 *
 * \class GSystemGDMLFactory
 * \brief Placeholder factory for GDML-based systems.
 *
 * Current status:
 * - Geometry loading is a stub that logs a message.
 * - Materials loading is a stub that logs a message.
 *
 * Intended future behavior:
 * - Parse a GDML file and create corresponding volumes/material mappings,
 *   or import a geometry and then synthesize GVolume records similarly to CAD import.
 *
 */
class GSystemGDMLFactory : public GSystemFactory
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GSystemFactory::GSystemFactory;

private:
	/**
	 * \brief Materials loading stub.
	 *
	 * \param system Target system.
	 *
	 * \details The current implementation does not load materials and only logs a placeholder message.
	 */
	void loadMaterials(GSystem* system) override;

	/**
	 * \brief Geometry loading stub.
	 *
	 * \param system Target system.
	 *
	 * \details The current implementation does not load geometry and only logs a placeholder message.
	 */
	void loadGeometry(GSystem* system) override;
};
