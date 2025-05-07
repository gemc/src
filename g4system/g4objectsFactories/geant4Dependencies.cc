// guts
#include "gutilities.h"

// g4system
#include "gsystemConventions.h"
#include "g4objectsFactory.h"


bool G4ObjectsFactory::checkSolidDependencies(const GVolume* s,
                                              std::unordered_map<std::string,
                                                                 G4Volume*>* g4s) {

	// checking if it's a copy, replica or solid operation – they are mutually exclusive
	std::string copyOf    = s->getCopyOf();
	std::string replicaOf = s->getReplicaOf();
	std::string solidsOpr = s->getSolidsOpr();

	string message;
	if (copyOf != UNINITIALIZEDSTRINGQUANTITY) {
		message = ", copyOf: " + copyOf;
	}
	else if (replicaOf != UNINITIALIZEDSTRINGQUANTITY) {
		message = ", replicaOf: " + replicaOf;
	}
	else if (solidsOpr != UNINITIALIZEDSTRINGQUANTITY) {
		message = ", solidsOpr: " + solidsOpr;
	}

	log->debug(NORMAL, className(), " checkSolidDependencies: checking dependencies for <",
		s->getName(), ">", message);

	/*──────────────────────────────────── copyOf: volumeName ───────────────────────────────────*/
	if (copyOf != UNINITIALIZEDSTRINGQUANTITY) {
		std::vector<std::string> copies = gutilities::getStringVectorFromString(copyOf);

		if (copies.size() == 2) {
			// first string must be copyOf
			if (copies[0] == "copyOf:") {
				// checking if the copy solid exists
				log->info(2, "<", s->getName(), "> is a copy of <", copies[1], ">");

				if (getSolidFromMap(copies[1], g4s) != nullptr) {
					log->info(2, "which already exists");
					return true;
				}
				else {
					log->info(2, "which does not exist yet");
					return false;
				}
			}
			else {
				log->warning("Did you intend to make a copy? The first token must be \"copyOf:\" but you have " +
				              copies[0]);
				return false;
			}
		}
		else {
			log->warning("Did you intend to make a copy? Syntax is \"copyOf:\" volumeName, but you have " + copyOf);
			return false;
		}
	}

	/*──────────────────────────────────── replicaOf – (not yet handled) ─────────────────────────*/
	else if (replicaOf != UNINITIALIZEDSTRINGQUANTITY) {
		// TODO: implement replica logic
	}

	/*──────────────────────────────────── Boolean solid operations ──────────────────────────────*/
	else if (solidsOpr != UNINITIALIZEDSTRINGQUANTITY) {
		std::vector<std::string> solidOperations =
			gutilities::getStringVectorFromString(solidsOpr);

		if (solidOperations.size() == 3) {
			if (solidOperations[1] == "+" || solidOperations[1] == "-" || solidOperations[1] == "*") {
				// checking if the operand solids exist
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
	// PRAGMA TODO: check material here
	return true;
}

/**
 * @brief Verify that both the *candidate* logical and its *mother* logical
 *        volume exist in the map before attempting to build a physical volume.
 *
 * @return `true` if the logical prerequisites are satisfied, otherwise `false`.
 */
bool G4ObjectsFactory::checkPhysicalDependencies(const GVolume* s,
                                                 std::unordered_map<std::string,
                                                                    G4Volume*>* g4s) {
	std::string vname      = s->getG4Name();
	std::string motherName = s->getG4MotherName();

	/* candidate GVolume must exist */
	if (g4s->find(vname) == g4s->end()) {
		log->info(2, "dependencies: ", vname, " not found in gvolume map yet.");
		return false;
	}

	/* candidate logical must exist */
	if (getLogicalFromMap(vname, g4s) == nullptr) {
		log->info(2, "dependencies: ", vname, " logical volume not found yet.");
		return false;
	}

	/* mother logical must exist (unless WORLD volume) */
	if (motherName != MOTHEROFUSALL &&
	    getLogicalFromMap(motherName, g4s) == nullptr) {
		log->info(2, "dependencies: ", vname,
		           " mother <", motherName, "> logical volume not found yet.");
		return false;
	}

	/* everything satisfied – emit verbose trace */
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
