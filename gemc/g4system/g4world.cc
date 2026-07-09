// g4world.cc : implementation of the Geant4 world builder and material initialization.
/**
 * \file   g4world.cc
 * @ingroup g4system_geometry
 * \brief  Implementation of G4World, including factory creation, material building, and volume dependency resolution.
 *
 * @details
 * Header documentation in \c g4world.h is authoritative. This file focuses on implementation details and
 * uses short, non-Doxygen comments to explain complex control-flow blocks (dependency retries, stall detection).
 */

// gemc
#include "g4world.h"
#include "gfactory.h"

// g4system
#include "g4system_options.h"
#include "g4systemConventions.h"
#include "gsystemConventions.h"
#include "g4objectsFactories/g4native/g4NativeObjectsFactory.h"
#include "g4objectsFactories/cad/cadSystemFactory.h"


// c++
#include <vector>

G4World::G4World(const GWorld *gworld, const std::shared_ptr<GOptions> &gopts)
	: GBase(gopts, G4SYSTEM_LOGGER) {
	auto gsystemMap = gworld->getSystemsMap();

	// Phase 1: create and initialize a Geant4 object factory for each system.
	// The factory provides solid/logical/physical creation for volumes in that system.
	createG4SystemFactory(gopts,
	                      gsystemMap,
	                      gopts->getScalarString("useBackupMaterial"),
	                      gopts->getScalarInt("check_overlaps")
	);

	// Phase 2: build all materials across systems, resolving dependencies iteratively.
	buildMaterials(gsystemMap);

	// Phase 3: ensure common isotopes/elements/materials exist (used by typical configurations).
	buildDefaultMaterialsElementsAndIsotopes();

	// Phase 4: build volumes. Some volumes depend on mothers that may not exist yet,
	// so we iterate until the remaining list becomes empty or the dependency resolution stalls.
	std::vector<GVolume *> thisIterationRemainingVolumes;
	unsigned long previousRemainingVolumes = 0;

	do {
		thisIterationRemainingVolumes.clear();

		// Loop over all systems and attempt to build all volumes in each system.
		for (auto &[systemName, gsystem]: *gsystemMap) {
			std::string g4Factory = g4FactoryNameFromSystemFactory(gsystem->getFactoryName());
			auto objectsFactory = get_factory(g4Factory);

			for (auto &[volumeName, gvolumePtr]: gsystem->getGVolumesMap()) {
				auto *gvolume = gvolumePtr.get();

				// Try to build; if dependencies are missing, remember it for the next iteration.
				if (!build_g4volume(gvolume, objectsFactory)) {
					// Only track volumes that are meant to exist; nonexistent volumes are skipped quietly.
					if (gvolume->getExistence()) {
						log->info(2, " >> adding volumeName <", volumeName, "> to the list of remaining volumes");
						thisIterationRemainingVolumes.push_back(gvolume);
					}
				}
			}

			// Diagnostic listing of the volumes that could not be built due to missing mothers.
			if (!thisIterationRemainingVolumes.empty()) {
				log->info(2, "G4World: ", systemName, " : ",
				          thisIterationRemainingVolumes.size(),
				          " remaining motherless g4volumes to be built:");
				for (auto *gvolumeLeft: thisIterationRemainingVolumes) {
					log->info(2, "G4World: ", gvolumeLeft->getName(),
					          " with mother <", gvolumeLeft->getG4MotherName(), "> ");
				}
			}
		}

		// Dependency-stall detection: error only when the remaining count fails to strictly
		// decrease across iterations (a strictly smaller count means progress was made).
		if (previousRemainingVolumes != 0 && !thisIterationRemainingVolumes.empty() &&
		    thisIterationRemainingVolumes.size() >= previousRemainingVolumes) {
			for (auto *gvolumeLeft: thisIterationRemainingVolumes) {
				log->warning(" >> ", gvolumeLeft->getName(),
				             " with mother <", gvolumeLeft->getG4MotherName(), "> not built");
			}
			log->error(ERR_G4DEPENDENCIESNOTSOLVED,
			           "dependencies are not being resolved: their number should diminish. "
			           "Above are the outstanding gvolumes");
		}
		previousRemainingVolumes = thisIterationRemainingVolumes.size();
	} while (!thisIterationRemainingVolumes.empty());

	// Phase 5: build optical surfaces (mirrors), now that all logical/physical volumes exist.
	buildOpticalSurfaces(gsystemMap);

	// Optional diagnostic output: list known materials from the Geant4 NIST manager.
	if (gopts->getSwitch("showPredefinedMaterials")) { G4NistManager::Instance()->ListMaterials("all"); }

	// Optional diagnostic output: print materials used in the simulation.
	if (gopts->getSwitch("printSystemsMaterials")) {
		auto matTable = (G4MaterialTable *) G4Material::GetMaterialTable();
		for (auto thisMat: *matTable) {
			log->info(0, 2, "G4World: GEMC Material: <", thisMat->GetName(), ">, density: ",
			          thisMat->GetDensity() / (CLHEP::g / CLHEP::cm3), "g/cm3");

			// Print each component; negative/zero values mean "fractional mass", positive means "number of atoms".
			for (auto &[material, component]: thisMat->GetMatComponents()) {
				if (component > 0.0) {
					log->info(0, "element", material->GetName(), "number of atoms: ", component);
				} else { log->info(0, "element", material->GetName(), "fractional mass: ", component); }
			}
		}
	}
}

/*──────────────────────── look-ups ──────────────────────────*/

const G4Volume *G4World::getG4Volume(const std::string &volumeName) const {
	auto it = g4volumesMap.find(volumeName);
	return (it != g4volumesMap.end()) ? it->second : nullptr;
}

void G4World::setFieldManagerForVolume(const std::string &volumeName,
                                       G4FieldManager *fm,
                                       bool forceToAllDaughters) {
	auto it = g4volumesMap.find(volumeName);
	if (it != g4volumesMap.end()) it->second->setFieldManager(fm, forceToAllDaughters);
}

// ---- g4FactoryNameFromSystemFactory -----------------------------------------------------------
std::string G4World::g4FactoryNameFromSystemFactory(const std::string &factory) const {
	// Map GEMC system factory labels to g4system object factory labels.
	if (factory == GSYSTEMASCIIFACTORYLABEL ||
	    factory == GSYSTEMSQLITETFACTORYLABEL ||
	    factory == GSYSTEMMYSQLTFACTORYLABEL) { return G4SYSTEMNATFACTORY; } else if (
		factory == GSYSTEMCADTFACTORYLABEL) { return G4SYSTEMCADFACTORY; } else {
		log->error(ERR_G4SYSTEMFACTORYNOTFOUND,
		           "gsystemFactory factory <", factory, "> is not mapped to any G4SystemFactory");
	}
}

void G4World::createG4SystemFactory(const std::shared_ptr<GOptions> &gopts,
                                    SystemMap *gsystemsMap,
                                    const std::string &backup_material,
                                    int check_overlaps) {
	// Instantiate a manager used to register and create factories.
	GManager manager(gopts);

	// Creating the native factory no matter what (it is the default for ASCII/SQLite/MySQL systems).
	log->info(2, "G4World: registering default factory <", G4SYSTEMNATFACTORY, ">");
	manager.RegisterObjectFactory<G4NativeSystemFactory>(G4SYSTEMNATFACTORY, gopts);

	// Register factories based on the system factory label, then create/initialize them lazily.
	for (auto &[gsystemName, gsystem]: *gsystemsMap) {
		std::string factory = gsystem->getFactoryName();
		std::string g4Factory = g4FactoryNameFromSystemFactory(factory);

		log->info(2, "G4World: creating factory <", g4Factory, "> to for system <", gsystemName, ">");

		// Register needed factory types:
		// this will always be false for the default native label because it was registered above.
		if (factory == GSYSTEMASCIIFACTORYLABEL || factory == GSYSTEMSQLITETFACTORYLABEL ||
		    factory == GSYSTEMMYSQLTFACTORYLABEL) {
			if (g4systemFactory.find(g4Factory) == g4systemFactory.end()) {
				manager.RegisterObjectFactory<G4NativeSystemFactory>(g4Factory, gopts);
			}
		} else if (factory == GSYSTEMCADTFACTORYLABEL) {
			if (g4systemFactory.find(GSYSTEMCADTFACTORYLABEL) == g4systemFactory.end()) {
				manager.RegisterObjectFactory<G4CadSystemFactory>(g4Factory, gopts);
			}
		}

		// Create and initialize the concrete factory instance once per label.
		if (g4systemFactory.find(g4Factory) == g4systemFactory.end()) {
			g4systemFactory[g4Factory] = manager.CreateObject<G4ObjectsFactory>(g4Factory);
			g4systemFactory[g4Factory]->initialize_context(check_overlaps, backup_material);
		}
	}
}

void G4World::buildMaterials(SystemMap *system_map) {
	// Build materials across all systems. Some materials may depend on other materials/elements,
	// so we iterate until all dependencies are resolved or the resolution stalls.
	std::vector<GMaterial *> thisIterationRemainingMaterials;
	unsigned long previousRemainingMaterials = 0;
	do {
		thisIterationRemainingMaterials.clear();

		for (const auto &[systemName, system]: *system_map) {
			// Loop over the material map in each system and attempt to build each material.
			for (const auto &[gmaterialName, gmaterialPtr]: system->getGMaterialMap()) {
				if (createG4Material(gmaterialPtr) == false) {
					thisIterationRemainingMaterials.push_back(gmaterialPtr.get());
				}
			}
		}

		// Dependency-stall detection for material building: error only when the remaining
		// count fails to strictly decrease across iterations.
		if (previousRemainingMaterials != 0 && !thisIterationRemainingMaterials.empty() &&
		    thisIterationRemainingMaterials.size() >= previousRemainingMaterials) {
			for (auto &gmaterialLeft: thisIterationRemainingMaterials) { log->warning(gmaterialLeft->getName()); }
			log->error(ERR_G4DEPENDENCIESNOTSOLVED,
			           "Dependencies are not being resolved: their number should diminish. Above are the Outstanding gmaterials");
		}
		previousRemainingMaterials = thisIterationRemainingMaterials.size();
	} while (!thisIterationRemainingMaterials.empty());
}

bool G4World::build_g4volume(const GVolume *s, G4ObjectsFactory *objectsFactory) {
	log->info(2, "G4World: using factory <", objectsFactory->className(),
	          "> to build g4volume <", s->getG4Name(), ">");

	return objectsFactory->build_g4volume(s, &g4volumesMap);
}
