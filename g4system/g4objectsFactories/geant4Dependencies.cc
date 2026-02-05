// geant4Dependencies.cc : dependency checks used by factory default implementations.
/**
 * @file   geant4Dependencies.cc
 * @ingroup g4system_geometry
 * @brief  Dependency checks used by g4system factories before constructing solids and placements.
 *
 * @details
 * Header documentation in \c g4objectsFactory.h is authoritative. This file implements:
 * - solid dependency checks (copy-of and boolean-operand existence)
 * - physical dependency checks (candidate logical, copy-source logical, and mother logical existence)
 */

// guts
#include "gutilities.h"

// g4system
#include "gsystemConventions.h"
#include "g4objectsFactory.h"

bool G4ObjectsFactory::checkSolidDependencies(const GVolume* s,
                                              std::unordered_map<std::string,
                                                                 G4Volume*>* g4s) {
	// Dependency check applies only to solids that rely on other solids (copy/boolean operations).
	// Ordinary primitives have no extra prerequisites.
	std::string copyOf    = s->getCopyOf();
	std::string solidsOpr = s->getSolidsOpr();
	std::string gsystem   = s->getSystem();

	std::string message;

	if (copyOf != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) { message = ", copyOf: " + copyOf; }
	else if (solidsOpr != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) { message = ", solidsOpr: " + solidsOpr; }

	log->debug(NORMAL, className(), " checkSolidDependencies: checking dependencies for <",
	           s->getName(), ">", message);

	/*──────────────────────────────────── copyOf: volumeName ───────────────────────────────────*/
	if (copyOf != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) {
		auto volume_copy = gsystem + "/" + copyOf;
		if (getSolidFromMap(volume_copy, g4s) != nullptr) {
			log->info(2, "<", s->getName(), "> is a copy of <", volume_copy, ">, which already exists");
			return true;
		}
		log->info(2, "<", s->getName(), "> is a copy of <", volume_copy,
		          ">, which already exists, which does not exist yet");
		return false;
	}

	/*──────────────────────────────────── Boolean solid operations ──────────────────────────────*/
	else if (solidsOpr != "" && solidsOpr != UNINITIALIZEDSTRINGQUANTITY) {
		// The solids operation is expected to be tokenized into: left operand, operator, right operand.
		std::vector<std::string> solidOperations =
			gutilities::getStringVectorFromString(solidsOpr);

		if (solidOperations.size() == 3) {
			// Supported operators: + (union), - (subtraction), * (intersection).
			if (solidOperations[1] == "+" || solidOperations[1] == "-" || solidOperations[1] == "*") {
				// Operand solids must exist before the boolean solid can be created.
				if (getSolidFromMap(solidOperations[0], g4s) != nullptr &&
					getSolidFromMap(solidOperations[2], g4s) != nullptr)
					return true;
			}
			return false;
		}
		return false;
	}

	/*────────────────────────────────────    default path     ──────────────────────────────────*/
	return true; // ordinary primitive – no extra prerequisites
}

bool G4ObjectsFactory::checkLogicalDependencies([[maybe_unused]] const GVolume* s,
                                                [[maybe_unused]] std::unordered_map<std::string,
	                                                G4Volume*>* g4s) {
	// Placeholder for future logical checks (materials/sensitive detector prerequisites).
	return true;
}

// Verify that both the candidate logical volume and its mother logical volume exist before placement.
// Header documentation is authoritative; this implementation comment is intentionally brief.
bool G4ObjectsFactory::checkPhysicalDependencies(const GVolume* s,
                                                 std::unordered_map<std::string,
                                                                    G4Volume*>* g4s) {
	std::string vname      = s->getG4Name();
	std::string motherName = s->getG4MotherName();

	// Candidate wrapper must exist in the map before we can reason about its logical/physical state.
	if (g4s->find(vname) == g4s->end()) {
		log->info(2, "dependencies: ", vname, " not found in gvolume map yet.");
		return false;
	}

	// Candidate logical must exist (or be available through a copy source).
	if (getLogicalFromMap(vname, g4s) == nullptr) {
		// If it is a copy, require that the source logical volume exists.
		std::string copyOf = s->getCopyOf();
		if (copyOf != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) {
			auto gsystem     = s->getSystem();
			auto volume_copy = gsystem + "/" + copyOf;
			if (getLogicalFromMap(volume_copy, g4s) == nullptr) {
				log->info(2, "dependencies: copy ", volume_copy, " logical volume not found yet.");
				return false;
			}
		}
		else {
			log->info(2, "dependencies: ", vname, " logical volume not found yet.");
			return false;
		}
	}

	// Mother logical must exist unless this is the world volume.
	if (motherName != MOTHEROFUSALL && getLogicalFromMap(motherName, g4s) == nullptr) {
		log->info(2, "dependencies: ", vname,
		          " mother <", motherName, "> logical volume not found yet.");
		return false;
	}

	// Everything satisfied – emit verbose trace.
	if (motherName != MOTHEROFUSALL) {
		log->info(2, "dependencies: <", vname, "> and mother <", motherName,
		          "> logical volumes are found. Ready to build or get physical volume.");
	}
	else {
		log->info(2, "dependencies: <", vname,
		          "> logical volume is found. Ready to build or get physical volume.");
	}
	return true;
}
