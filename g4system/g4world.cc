// g4world.cc : implementation of the Geant4 world builder and material initialization.

// gemc
#include "g4world.h"
#include "gfactory.h"

// g4system
#include "g4system_options.h"
#include "g4system/g4systemConventions.h"
#include "gsystemConventions.h"
#include "g4objectsFactories/g4native/g4NativeObjectsFactory.h"
#include "g4objectsFactories/cad/cadSystemFactory.h"

// geant4
#include "G4MaterialTable.hh"
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4Isotope.hh"

// c++
#include <vector>

G4World::G4World(const GWorld* gworld, const std::shared_ptr<GOptions>& gopts)
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
	std::vector<GVolume*> thisIterationRemainingVolumes;
	unsigned long         allRemainingVolumes = 0;

	do {
		thisIterationRemainingVolumes.clear();

		// Loop over all systems and attempt to build all volumes in each system.
		for (auto& [systemName, gsystem] : *gsystemMap) {
			std::string g4Factory      = g4FactoryNameFromSystemFactory(gsystem->getFactoryName());
			auto        objectsFactory = get_factory(g4Factory);

			for (auto& [volumeName, gvolumePtr] : gsystem->getGVolumesMap()) {
				auto* gvolume = gvolumePtr.get();

				// Try to build; if dependencies are missing, remember it for the next iteration.
				if (!build_g4volume(gvolume, objectsFactory)) {
					// Only track volumes that are meant to exist; nonexistent volumes are skipped quietly.
					if (gvolume->getExistence()) {
						log->warning(" >> adding volumeName <", volumeName, "> to the list of remaining volumes");
						thisIterationRemainingVolumes.push_back(gvolume);
					}
				}
			}

			// Diagnostic listing of the volumes that could not be built due to missing mothers.
			if (!thisIterationRemainingVolumes.empty()) {
				log->info(2, "G4World: ", systemName, " : ",
				          thisIterationRemainingVolumes.size(),
				          " remaining motherless g4volumes to be built:");
				for (auto* gvolumeLeft : thisIterationRemainingVolumes) {
					log->info(2, "G4World: ", gvolumeLeft->getName(),
					          " with mother <", gvolumeLeft->getG4MotherName(), "> ");
				}
			}
		}

		// Dependency-stall detection:
		// If the number of remaining volumes does not decrease across iterations, dependencies are not solvable.
		if (allRemainingVolumes != 0 && !thisIterationRemainingVolumes.empty()) {
			if (allRemainingVolumes >= thisIterationRemainingVolumes.size()) {
				for (auto* gvolumeLeft : thisIterationRemainingVolumes) {
					log->warning(" >> ", gvolumeLeft->getName(),
					             " with mother <", gvolumeLeft->getG4MotherName(), "> not built");
				}
				log->error(ERR_G4DEPENDENCIESNOTSOLVED,
				           "dependencies are not being resolved: their number should diminish. "
				           "Above are the outstanding gvolumes");
			}
		}
		else { allRemainingVolumes = thisIterationRemainingVolumes.size(); }
	}
	while (!thisIterationRemainingVolumes.empty());

	// Optional diagnostic output: list known materials from the Geant4 NIST manager.
	if (gopts->getSwitch("showPredefinedMaterials")) { G4NistManager::Instance()->ListMaterials("all"); }

	// Optional diagnostic output: print materials used in the simulation.
	if (gopts->getSwitch("printSystemsMaterials")) {
		auto matTable = (G4MaterialTable*)G4Material::GetMaterialTable();
		for (auto thisMat : *matTable) {
			log->info(0, 2, "G4World: GEMC Material: <", thisMat->GetName(), ">, density: ",
			          thisMat->GetDensity() / (CLHEP::g / CLHEP::cm3), "g/cm3");

			// Print each component; negative/zero values mean "fractional mass", positive means "number of atoms".
			for (auto& [material, component] : thisMat->GetMatComponents()) {
				if (component > 0.0) { log->info(0, "element", material->GetName(), "number of atoms: ", component); }
				else { log->info(0, "element", material->GetName(), "fractional mass: ", component); }
			}
		}
	}
}


/*──────────────────────── look-ups ──────────────────────────*/

const G4Volume* G4World::getG4Volume(const std::string& volumeName) const {
	auto it = g4volumesMap.find(volumeName);
	return (it != g4volumesMap.end()) ? it->second : nullptr;
}

void G4World::setFieldManagerForVolume(const std::string& volumeName,
                                       G4FieldManager*    fm,
                                       bool               forceToAllDaughters) {
	auto it = g4volumesMap.find(volumeName);
	if (it != g4volumesMap.end()) it->second->setFieldManager(fm, forceToAllDaughters);
}


// ---- g4FactoryNameFromSystemFactory -----------------------------------------------------------
std::string G4World::g4FactoryNameFromSystemFactory(const std::string& factory) const {
	// Map GEMC system factory labels to g4system object factory labels.
	if (factory == GSYSTEMASCIIFACTORYLABEL ||
		factory == GSYSTEMSQLITETFACTORYLABEL ||
		factory == GSYSTEMMYSQLTFACTORYLABEL) { return G4SYSTEMNATFACTORY; }
	else if (factory == GSYSTEMCADTFACTORYLABEL) { return G4SYSTEMCADFACTORY; }
	else {
		log->error(ERR_G4SYSTEMFACTORYNOTFOUND,
		           "gsystemFactory factory <", factory, "> is not mapped to any G4SystemFactory");
	}
}

bool G4World::createG4Material(const std::shared_ptr<GMaterial>& gmaterial) {
	auto NISTman      = G4NistManager::Instance(); // material G4 Manager
	auto materialName = gmaterial->getName();

	// Only build the material if it is not already available in Geant4.
	auto g4material = NISTman->FindMaterial(materialName);
	if (g4material != nullptr) {
		log->info(2, "Material <", materialName, "> already exists in G4NistManager");
		return true;
	}

	auto components = gmaterial->getComponents();
	auto amounts    = gmaterial->getAmounts();
	bool isChemical = gmaterial->isChemicalFormula();

	// Scan material components:
	// return false if any component does not exist yet (caller will retry later).
	for (auto& componentName : components) {
		if (isChemical) {
			if (NISTman->FindOrBuildElement(componentName) == nullptr) {
				log->info(2, "Element <", componentName, ">, needed by ", materialName, ",  not found yet");
				return false;
			}
			else { log->info(2, "Element <", componentName, "> needed by ", materialName, " now found"); }
		}
		else {
			if (NISTman->FindOrBuildMaterial(componentName) == nullptr) {
				log->info(2, "Material <", componentName, ">, needed by ", materialName, ",  not found yet");
				return false;
			}
			else { log->info(2, "Material <", componentName, "> needed by ", materialName, " now found"); }
		}
	}

	// Build the composed material from its components.
	auto density                 = gmaterial->getDensity();
	g4materialsMap[materialName] = new G4Material(materialName, density * CLHEP::g / CLHEP::cm3,
	                                              static_cast<G4int>(components.size()));

	if (isChemical) {
		log->info(2, "Building material <", materialName, "> with components:");
		for (size_t i = 0; i < components.size(); i++) {
			log->info(2, "element <", components[i], "> with amount: ", amounts[i]);
		}

		for (size_t i = 0; i < components.size(); i++) {
			auto element = NISTman->FindOrBuildElement(components[i]);
			g4materialsMap[materialName]->AddElement(element, static_cast<G4int>(amounts[i]));
		}
	}
	else {
		log->info(2, "Building material <", materialName, "> with components:");
		for (size_t i = 0; i < components.size(); i++) {
			log->info(2, "material <", components[i], "> with fractional mass: ", amounts[i]);
		}

		for (size_t i = 0; i < components.size(); i++) {
			auto material = NISTman->FindOrBuildMaterial(components[i]);
			g4materialsMap[materialName]->AddMaterial(material, amounts[i]);
		}
	}

	return true;
}


void G4World::buildDefaultMaterialsElementsAndIsotopes() {
	// Create a small set of commonly-used isotopes/elements/materials if they are missing.
	// These are defined using Geant4 primitives and then registered in the local map for reference.
	int    Z, N;
	double a, d, T;


	// ----  Hydrogen

	// Hydrogen gas material definition (Hydrogen element + state/gas parameters).
	if (G4NistManager::Instance()->FindMaterial(HGAS_MATERIAL) == nullptr) {
		Z                             = 1;
		a                             = 1.01 * CLHEP::g / CLHEP::mole;
		d                             = 0.00275 * CLHEP::g / CLHEP::cm3;
		T                             = 50.0 * CLHEP::kelvin;
		auto Hydrogen                 = new G4Element(HYDROGEN_ELEMENT, HYDROGEN_ELEMENT, Z, a);
		g4materialsMap[HGAS_MATERIAL] = new G4Material(HGAS_MATERIAL,
		                                               d,
		                                               1,
		                                               kStateGas,
		                                               T);
		g4materialsMap[HGAS_MATERIAL]->AddElement(Hydrogen, 1);
	}
	log->info(2, "G4World: Hydrogen gas material <", HGAS_MATERIAL, "> created with density <", d, ">");


	// ----  Deuterium

	// Deuteron isotope and Deuterium element definition.
	if (G4NistManager::Instance()->FindOrBuildElement(DEUTERIUM_ELEMENT) == nullptr) {
		Z             = 1;
		N             = 2;
		a             = 2.0141018 * CLHEP::g / CLHEP::mole;
		auto Deuteron = new G4Isotope(DEUTERON_ISOTOPE, Z, N, a);

		// Deuterium element: isotope composition is explicitly set to the Deuteron isotope.
		Deuterium = new G4Element(DEUTERIUM_ELEMENT, DEUTERIUM_ELEMENT, 1);
		Deuterium->AddIsotope(Deuteron, 1);
	}
	log->info(2, "G4World: Deuterium element <", DEUTERIUM_ELEMENT, "> created with density <", d, ">");

	// Deuterium gas material.
	if (G4NistManager::Instance()->FindMaterial(DEUTERIUMGAS_MATERIAL) == nullptr) {
		d                                     = 0.000452 * CLHEP::g / CLHEP::cm3;
		T                                     = 294.25 * CLHEP::kelvin;
		g4materialsMap[DEUTERIUMGAS_MATERIAL] = new G4Material(DEUTERIUMGAS_MATERIAL,
		                                                       d,
		                                                       1,
		                                                       kStateGas,
		                                                       T);
		g4materialsMap[DEUTERIUMGAS_MATERIAL]->AddElement(Deuterium, 1);
	}
	log->info(2, "G4World: Deuterium gas material <", DEUTERIUMGAS_MATERIAL, "> created with density <", d, ">");

	// Liquid Deuterium material.
	if (G4NistManager::Instance()->FindMaterial(LD2_MATERIAL) == nullptr) {
		d                            = 0.169 * CLHEP::g / CLHEP::cm3;
		T                            = 22.0 * CLHEP::kelvin;
		g4materialsMap[LD2_MATERIAL] = new G4Material(LD2_MATERIAL,
		                                              d,
		                                              1,
		                                              kStateLiquid,
		                                              T);
		g4materialsMap[LD2_MATERIAL]->AddElement(Deuterium, 2);
	}
	log->info(2, "G4World: Liquid Deuterium material <", LD2_MATERIAL, "> created with density <", d, ">");

	// Ammonia (ND3) material definition.
	if (G4NistManager::Instance()->FindMaterial(ND3_MATERIAL) == nullptr) {
		Z                            = 7;
		a                            = 14.01 * CLHEP::g / CLHEP::mole;
		d                            = 1.007 * CLHEP::g / CLHEP::cm3;
		T                            = 1.0 * CLHEP::kelvin;
		auto Nitrogen                = new G4Element(NITRO_ELEMENT, NITRO_ELEMENT, Z, a);
		g4materialsMap[ND3_MATERIAL] = new G4Material(ND3_MATERIAL,
		                                              d,
		                                              2,
		                                              kStateLiquid,
		                                              T);
		g4materialsMap[ND3_MATERIAL]->AddElement(Nitrogen, 1);
		g4materialsMap[ND3_MATERIAL]->AddElement(Deuterium, 3);
	}
	log->info(2, "G4World: Ammonia material <", ND3_MATERIAL, "> created with density <", d, ">");

	// ---- Helium 3

	// Helion isotope and Helium3 element definition.
	if (G4NistManager::Instance()->FindOrBuildElement(HELIUM3_ELEMENT) == nullptr) {
		Z           = 2;
		N           = 3;
		a           = 3.0160293 * CLHEP::g / CLHEP::mole;
		auto Helion = new G4Isotope(HELION_ISOTOPE, Z, N, a);

		// Helium-3 element: isotope composition is explicitly set to the Helion isotope.
		Helium3 = new G4Element(HELIUM3_ELEMENT, HELIUM3_ELEMENT, 1);
		Helium3->AddIsotope(Helion, 1);
	}
	log->info(2, "G4World: Helium 3 element <", HELIUM3_ELEMENT, "> created with density <", d, ">");

	// Helium-3 gas material definition.
	if (G4NistManager::Instance()->FindMaterial(HELIUM3GAS_MATERIAL) == nullptr) {
		// Density at 21.1°C (70°F): 0.1650 kg/m3.
		d                                   = 0.1650 * CLHEP::mg / CLHEP::cm3;
		T                                   = 294.25 * CLHEP::kelvin;
		g4materialsMap[HELIUM3GAS_MATERIAL] = new G4Material(HELIUM3GAS_MATERIAL,
		                                                     d,
		                                                     1,
		                                                     kStateGas,
		                                                     T);
		g4materialsMap[HELIUM3GAS_MATERIAL]->AddElement(Helium3, 1);
	}
	log->info(2, "G4World: Helium 3 gas material <", HELIUM3GAS_MATERIAL, "> created with density <", d, ">");


	// ---- Tritium

	// Triton isotope and Tritium element definition.
	if (G4NistManager::Instance()->FindOrBuildElement(TRITIUM_ELEMENT) == nullptr) {
		Z           = 1;
		N           = 3;
		a           = 3.0160492 * CLHEP::g / CLHEP::mole;
		auto Triton = new G4Isotope(TRITON_ISOTOPE, Z, N, a);

		Tritium = new G4Element(TRITIUM_ELEMENT, TRITIUM_ELEMENT, 1);
		Tritium->AddIsotope(Triton, 1);
	}
	log->info(2, "G4World: Tritium element <", TRITIUM_ELEMENT, "> created with density <", d, ">");

	// Tritium gas material definition.
	if (G4NistManager::Instance()->FindMaterial(TRITIUMGAS_MATERIAL) == nullptr) {
		d                                   = 0.0034 * CLHEP::g / CLHEP::cm3;
		T                                   = 40.0 * CLHEP::kelvin;
		g4materialsMap[TRITIUMGAS_MATERIAL] = new G4Material(TRITIUMGAS_MATERIAL,
		                                                     d,
		                                                     1,
		                                                     kStateGas, T);
		g4materialsMap[TRITIUMGAS_MATERIAL]->AddElement(Tritium, 1);
	}
	log->info(2, "G4World: Tritium gas material <", TRITIUMGAS_MATERIAL, "> created with density <", d, ">");
}


void G4World::createG4SystemFactory(const std::shared_ptr<GOptions>& gopts,
                                    SystemMap*                       gsystemsMap,
                                    const std::string&               backup_material,
                                    int                              check_overlaps) {
	// Instantiate a manager used to register and create factories.
	GManager manager(gopts);

	// Creating the native factory no matter what (it is the default for ASCII/SQLite/MySQL systems).
	log->info(2, "G4World: registering default factory <", G4SYSTEMNATFACTORY, ">");
	manager.RegisterObjectFactory<G4NativeSystemFactory>(G4SYSTEMNATFACTORY, gopts);

	// Register factories based on the system factory label, then create/initialize them lazily.
	for (auto& [gsystemName, gsystem] : *gsystemsMap) {
		std::string factory   = gsystem->getFactoryName();
		std::string g4Factory = g4FactoryNameFromSystemFactory(factory);

		log->info(2, "G4World: creating factory <", g4Factory, "> to for system <", gsystemName, ">");

		// Register needed factory types:
		// this will always be false for the default native label because it was registered above.
		if (factory == GSYSTEMASCIIFACTORYLABEL || factory == GSYSTEMSQLITETFACTORYLABEL ||
			factory == GSYSTEMMYSQLTFACTORYLABEL) {
			if (g4systemFactory.find(g4Factory) == g4systemFactory.end()) {
				manager.RegisterObjectFactory<G4NativeSystemFactory>(g4Factory, gopts);
			}
		}
		else if (factory == GSYSTEMCADTFACTORYLABEL) {
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

void G4World::buildMaterials(SystemMap* system_map) {
	// Build materials across all systems. Some materials may depend on other materials/elements,
	// so we iterate until all dependencies are resolved or the resolution stalls.
	std::vector<GMaterial*> thisIterationRemainingMaterials;
	unsigned long           allRemainingMaterials = 0;
	do {
		thisIterationRemainingMaterials.clear();

		for (const auto& [systemName, system] : *system_map) {
			// Loop over the material map in each system and attempt to build each material.
			for (const auto& [gmaterialName, gmaterialPtr] : system->getGMaterialMap()) {
				if (createG4Material(gmaterialPtr) == false) {
					thisIterationRemainingMaterials.push_back(gmaterialPtr.get());
				}
			}
		}

		// Dependency-stall detection for material building.
		if (allRemainingMaterials != 0 && !thisIterationRemainingMaterials.empty()) {
			if (allRemainingMaterials >= thisIterationRemainingMaterials.size()) {
				for (auto& gmaterialLeft : thisIterationRemainingMaterials) { log->warning(gmaterialLeft->getName()); }
				log->error(ERR_G4DEPENDENCIESNOTSOLVED,
				           "Dependencies are not being resolved: their number should diminish. Above are the Outstanding gmaterials");
			}
		}
		else { allRemainingMaterials = thisIterationRemainingMaterials.size(); }
	}
	while (!thisIterationRemainingMaterials.empty());
}


bool G4World::build_g4volume(const GVolume* s, G4ObjectsFactory* objectsFactory) {
	log->info(2, "G4World: using factory <", objectsFactory->className(),
	          "> to build g4volume <", s->getG4Name(), ">");

	return objectsFactory->build_g4volume(s, &g4volumesMap);
}
