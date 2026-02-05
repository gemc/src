#pragma once

// gsystem
#include "systemFactory.h"

/**
 * \ingroup gemc_gsystem_factories_cad
 *
 * \class GSystemCADFactory
 * \brief Load a system from CAD assets (e.g. STL) found in a directory.
 *
 * Geometry loading behavior:
 * - The system file path is resolved against \c possibleLocationOfFiles.
 * - The directory is scanned for \c .stl files.
 * - Each STL is imported as a volume via \ref GSystem::addVolumeFromFile "addVolumeFromFile()".
 *
 * Optional YAML modifier hook:
 * - If a file named \c cad__<variation>.yaml is found in the directory, it is parsed.
 * - The current implementation logs lookups; a block of commented code shows intended
 *   application of per-volume overrides (shift/tilt/exists/mother/color/material/digitization/identifier).
 *
 * \note Materials loading is currently empty for CAD-based systems.
 */
class GSystemCADFactory : public GSystemFactory
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GSystemFactory::GSystemFactory;

private:
	/// \brief CAD factory does not currently load materials (placeholder).
	void loadMaterials(GSystem* system) override;

	/// \brief Scan directory for STL files and import them into the system.
	void loadGeometry(GSystem* system) override;
};
