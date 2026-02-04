#pragma once

// gsystem
#include "systemFactory.h"

// file types
#define GTEXTGEOMTYPE "__geometry_"
#define GTEXTMATSTYPE "__materials_"
#define GTEXTMIRSTYPE "__mirrors_"

/**
 * \class GSystemTextFactory
 * \brief Load a system from ASCII text files.
 *
 * File naming convention:
 * - Base name is \c system->getFilePath()
 * - For geometry:  <base> + GTEXTGEOMTYPE + <variation> + ".txt"
 * - For materials: <base> + GTEXTMATSTYPE + <variation> + ".txt"
 *
 * Search logic:
 * - The factory tries the default filename in the current working directory.
 * - If not found, it tries each entry in \c possibleLocationOfFiles.
 *
 * Behavior differences:
 * - Geometry file is mandatory (unless annotation "mats_only" is set).
 * - Materials file is optional.
 */
class GSystemTextFactory : public GSystemFactory
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GSystemFactory::GSystemFactory;

private:
	void loadMaterials(GSystem* system) override;

	void loadGeometry(GSystem* system) override;

	/**
	 * \brief Return an input stream for a system text file, searching possible locations.
	 *
	 * \param system Target system descriptor.
	 * \param SYSTEMTYPE One of the file-type macros:
	 * - GTEXTGEOMTYPE
	 * - GTEXTMATSTYPE
	 * - GTEXTMIRSTYPE (reserved for future use)
	 *
	 * \return Pointer to an open input stream, or \c nullptr if optional file is not found.
	 *
	 * \note The returned stream is heap-allocated and must be closed by the caller.
	 */
	std::ifstream* gSystemTextFileStream(
		GSystem* system, const std::string& SYSTEMTYPE); // SYSTEMTYPE one of file types above
};
