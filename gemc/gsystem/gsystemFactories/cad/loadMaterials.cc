/**
* \file loadMaterials.cc
 * \brief Materials loading stub for GSystemCADFactory.
 *
 * See systemCadFactory.h for API docs.
 */

// gsystem
#include "systemCadFactory.h"

void GSystemCADFactory::loadMaterials([[maybe_unused]] GSystem* system) {
	// CAD workflow typically relies on existing materials (e.g. from a global database)
	// or assigns materials via subsequent modifiers. This is currently a stub.
	log->info(0, "in empty GSystemCADFactory loadMaterials");
}
