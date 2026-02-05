#pragma once

// gsystem
#include "systemFactory.h"

// file types
#define GTEXTGEOMTYPE "__geometry_"
#define GTEXTMATSTYPE "__materials_"
#define GTEXTMIRSTYPE "__mirrors_"

/**
 * \ingroup gemc_gsystem_factories_text
 *
 * \class GSystemTextFactory
 * \brief Load a system from ASCII text files.
 *
 * File naming convention:
 * - Base name is \c system->getFilePath()
 * - For geometry:  \c base + GTEXTGEOMTYPE + variation + ".txt"
 * - For materials: \c base + GTEXTMATSTYPE + variation + ".txt"
 *
 * Search logic:
 * - The factory tries the default filename in the current working directory.
 * - If not found, it tries each entry in \c possibleLocationOfFiles in order.
 *
 * Behavior differences:
 * - Geometry file is mandatory unless the system annotation is \c "mats_only".
 * - Materials file is optional.
 *
 * Parsing:
 * - Each non-empty line is split using \c '|' as delimiter.
 * - The resulting positional fields are forwarded to the system builder methods.
 */
class GSystemTextFactory : public GSystemFactory
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GSystemFactory::GSystemFactory;

private:
	/**
	 * \brief Load materials from the system materials file (optional).
	 *
	 * \param system Target system to populate.
	 *
	 * \details If the materials file is not found, the method returns without error.
	 */
	void loadMaterials(GSystem* system) override;

	/**
	 * \brief Load geometry from the system geometry file (mandatory unless \c "mats_only").
	 *
	 * \param system Target system to populate.
	 *
	 * \details If the geometry file is missing and the annotation is not \c "mats_only",
	 * the helper used to open the stream triggers an error.
	 */
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
	 * \return Pointer to an open input stream, or \c nullptr if the optional file is not found.
	 *
	 * \details Ownership:
	 * - The returned stream is heap-allocated.
	 * - The caller is responsible for closing the stream.
	 *
	 * Error handling:
	 * - For geometry, failure to locate a file triggers an error unless \c system->getAnnotations() is \c "mats_only".
	 * - For materials, failure to locate a file is treated as "no materials provided".
	 */
	std::ifstream* gSystemTextFileStream(GSystem* system, const std::string& SYSTEMTYPE);
};
