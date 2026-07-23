#pragma once

// gsystem
#include <gemc/gsystem/gsystemFactories/systemFactory.h>

/**
 * \ingroup gemc_gsystem_factories_cad
 *
 * \class GSystemCADFactory
 * \brief Load a system from CAD assets (e.g. STL) whose definitions live in the sqlite database.
 *
 * Geometry loading behavior:
 * - The system file path is resolved against \c possibleLocationOfFiles to locate the mesh directory.
 * - The directory is scanned for \c .stl and \c .ply files, indexed by filename stem.
 * - The list of volumes to load - and their metadata (material, position, rotation, sensitivity, ...) -
 *   comes from the \c geometry table (rows matching experiment/system/variation/run). Only meshes whose
 *   stem matches a database row are imported; STL/PLY files present on disk but absent from the database
 *   are ignored. The mesh path is resolved against the directory and written into the volume description
 *   for the g4 CAD builder.
 *
 * Definitions are authored in a single YAML/JSON file and uploaded with \c "gemc-sqlite -cad" (pygemc),
 * which replaces the legacy clas12Tags \c cad_<variation>.gxml mechanism.
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

	/// \brief Load optical-surface (mirror) definitions from the sqlite database.
	void loadMirrors(GSystem* system) override;

	/// \brief Scan directory for STL files and import them into the system.
	void loadGeometry(GSystem* system) override;
};
