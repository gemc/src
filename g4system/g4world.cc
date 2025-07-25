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

G4World::G4World(const GWorld* gworld, GOptions* gopts)
	: log(std::make_shared<GLogger>(gopts, GSYSTEM_LOGGER, "G4World Constructor")) {

	log->debug(CONSTRUCTOR, "G4World");

	auto gsystemMap = gworld->getSystemsMap();
	createG4SystemFactory(
	                      gsystemMap,
	                      gopts->getScalarString("useBackupMaterial"),
	                      gopts->getScalarInt("check_overlaps")
	                     );

	buildMaterials(gsystemMap);
	buildDefaultMaterialsElementsAndIsotopes();

	// get the gsystem named ROOTWORLDGVOLUMENAME from the map
	auto rootSystemIt = gsystemMap->find(ROOTWORLDGVOLUMENAME);
	if (rootSystemIt == gsystemMap->end()) {
		log->error(ERR_G4SYSTEMFACTORYNOTFOUND,
		           "G4World: ROOT system <", ROOTWORLDGVOLUMENAME, "> not found in the map");
	}
	else { log->info(2, "G4World: ROOT system <", rootSystemIt->first, "> found in the map"); }
	auto rootSystem     = rootSystemIt->second.get();
	auto objectsFactory = get_factory(G4SYSTEMNATFACTORY);

	for (auto& [volumeName, gvolumePtr] : rootSystem->getGVolumesMap()) {
		if (volumeName == ROOTWORLDGVOLUMENAME) {
			auto* gvolume = gvolumePtr.get();
			if (!build_g4volume(gvolume, objectsFactory)) {
				if (gvolume->getExistence())
					log->error(ERR_G4VOLUMEBUILDFAILED,
					           "G4World: ROOT volume <", gvolume->getName(), "> not built");
			}
		}
	}

	// every volume not built (due to dependencies) increments allRemainingVolumes
	std::vector<GVolume*> thisIterationRemainingVolumes;
	unsigned long         allRemainingVolumes = 0;

	do {
		thisIterationRemainingVolumes.clear();

		// loop over systems in the gsystemsMap
		for (auto& [systemName, gsystem] : *gsystemMap) {
			std::string g4Factory = g4FactoryNameFromSystemFactory(gsystem->getFactoryName());
			objectsFactory   = get_factory(g4Factory);

			for (auto& [volumeName, gvolumePtr] : gsystem->getGVolumesMap()) {
				auto* gvolume = gvolumePtr.get();;
				// try to build; remember the ones that still have missing mothers
				if (!build_g4volume(gvolume, objectsFactory)) {
					if (gvolume->getExistence())
						thisIterationRemainingVolumes.push_back(gvolume);
				}
			}

			// unchanged diagnostic logging for remaining volumes ...
			if (!thisIterationRemainingVolumes.empty()) {
				log->info(2, "G4World: ", systemName, " : ",
				          thisIterationRemainingVolumes.size(),
				          " remaining motherless g4volumes to be built:");
				for (auto* gvolumeLeft : thisIterationRemainingVolumes) {
					log->info(2, "G4World: ", gvolumeLeft->getName(),
					          " with mother <", gvolumeLeft->getG4MotherName(), ">");
				}
			}
		}

		// unchanged dependency‑stall check ...
		if (allRemainingVolumes != 0 && !thisIterationRemainingVolumes.empty()) {
			if (allRemainingVolumes >= thisIterationRemainingVolumes.size()) {
				for (auto* gvolumeLeft : thisIterationRemainingVolumes) {
					log->warning(" >> ", gvolumeLeft->getName(),
					             " with mother <", gvolumeLeft->getG4MotherName(), ">");
				}
				log->error(ERR_G4DEPENDENCIESNOTSOLVED,
				           "dependencies are not being resolved: their number should diminish. "
				           "Above are the outstanding gvolumes");
			}
		}
		else { allRemainingVolumes = thisIterationRemainingVolumes.size(); }
	}
	while (!thisIterationRemainingVolumes.empty());


	if (gopts->getSwitch("showPredefinedMaterials")) { G4NistManager::Instance()->ListMaterials("all"); }

	if (gopts->getSwitch("printSystemsMaterials")) {
		auto matTable = (G4MaterialTable*)G4Material::GetMaterialTable();
		for (auto thisMat : *matTable) {
			log->info(0, 2, "G4World: GEMC Material: <", thisMat->GetName(), ">, density: ",
			          thisMat->GetDensity() / (CLHEP::g / CLHEP::cm3), "g/cm3");

			for (auto& [material, component] : thisMat->GetMatComponents()) {
				if (component > 0.0) { log->info(0, "element", material->GetName(), "number of atoms: ", component); }
				else { log->info(0, "element", material->GetName(), "fractional mass: ", component); }
			}


			// auto nelements = thisMat->GetNumberOfElements();
			// for (size_t e = 0; e < nelements; e++) {
			// 	auto element = thisMat->GetElement(e);
			// 	log->info(0, "element ", element->GetName(), " number of atoms: ", thisMat->GetFractionVector()[e]);
			// }
		}
		log->info(0, *(G4Material::GetMaterialTable()));
	}
}


/*──────────────────────── look‑ups ──────────────────────────*/

const G4Volume* G4World::getG4Volume(const std::string& volumeName) const {
	auto it = g4volumesMap.find(volumeName);
	return (it != g4volumesMap.end()) ? it->second : nullptr;
}

void G4World::setFieldManagerForVolume(const std::string&   volumeName,
                                       G4FieldManager* fm,
                                       bool            forceToAllDaughters) {
	auto it = g4volumesMap.find(volumeName);
	if (it != g4volumesMap.end()) it->second->setFieldManager(fm, forceToAllDaughters);
}

/*──────────────────────── helper bodies (UNCHANGED LOGIC, pointer replaced) ───────────────────*/

// ---- g4FactoryNameFromSystemFactory -----------------------------------------------------------
std::string G4World::g4FactoryNameFromSystemFactory(const std::string& factory) const {
	if (factory == GSYSTEMASCIIFACTORYLABEL || factory == GSYSTEMSQLITETFACTORYLABEL ||
	    factory == GSYSTEMMYSQLTFACTORYLABEL) { return G4SYSTEMNATFACTORY; }
	else if (factory == GSYSTEMCADTFACTORYLABEL) { return G4SYSTEMCADFACTORY; }
	else {
		log->error(ERR_G4SYSTEMFACTORYNOTFOUND,
		           "gsystemFactory factory <", factory, "> is not mapped to any G4SystemFactory");
	}
}

bool G4World::createG4Material(const std::unique_ptr<GMaterial>& gmaterial) {
	auto NISTman      = G4NistManager::Instance(); // material G4 Manager
	auto materialName = gmaterial->getName();

	// only build the material if it's not found in geant4
	auto g4material = NISTman->FindMaterial(materialName);
	if (g4material != nullptr) {
		log->info(2, "Material <", materialName, "> already exists in G4NistManager");

		return true;
	}

	auto components = gmaterial->getComponents();
	auto amounts    = gmaterial->getAmounts();
	bool isChemical = gmaterial->isChemicalFormula();

	// scan material components
	// return false if any component does not exist yet
	for (auto& componentName : components) {
		if (isChemical) {
			if (NISTman->FindOrBuildElement(componentName) == nullptr) {
				log->info(2, "Element <", componentName, ">, needed by ", materialName, ",  not found yet");
				return false;
			}
			else { log->info(20, "Element <", componentName, "> needed by ", materialName, " now found"); }
		}
		else {
			if (NISTman->FindOrBuildMaterial(componentName) == nullptr) {
				log->info(2, "Material <", componentName, ">, needed by ", materialName, ",  not found yet");
				return false;
			}
			else { log->info(20, "Material <", componentName, "> needed by ", materialName, " now found"); }
		}
	}

	// building material from components
	auto density                 = gmaterial->getDensity();
	g4materialsMap[materialName] = new G4Material(materialName, density * CLHEP::g / CLHEP::cm3,
	                                              static_cast<G4int>(components.size()));

	if (isChemical) {
		log->info(2, "Building material <", materialName, "> with components:");
		for (size_t i = 0; i < components.size(); i++) { log->info(2, "element <", components[i], "> with amount: ", amounts[i]); }

		for (size_t i = 0; i < components.size(); i++) {
			auto element = NISTman->FindOrBuildElement(components[i]);
			g4materialsMap[materialName]->AddElement(element, static_cast<G4int>(amounts[i]));
		}
	}
	else {
		log->info(2, "Building material <", materialName, "> with components:");
		for (size_t i = 0; i < components.size(); i++) { log->info(2, "material <", components[i], "> with fractional mass: ", amounts[i]); }

		for (size_t i = 0; i < components.size(); i++) {
			auto material = NISTman->FindOrBuildMaterial(components[i]);
			g4materialsMap[materialName]->AddMaterial(material, amounts[i]);
		}
	}

	return true;
}


void G4World::buildDefaultMaterialsElementsAndIsotopes() {
	// isotopes not yet defined, defining them for the first time

	int    Z, N;
	double a, d, T;


	// ----  Hydrogen

	// hydrogen gas
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

	// Deuteron isotope
	if (G4NistManager::Instance()->FindOrBuildElement(DEUTERIUM_ELEMENT) == nullptr) {
		Z             = 1;
		N             = 2;
		a             = 2.0141018 * CLHEP::g / CLHEP::mole;
		auto Deuteron = new G4Isotope(DEUTERON_ISOTOPE, Z, N, a);

		// Deuterium element
		Deuterium = new G4Element(DEUTERIUM_ELEMENT, DEUTERIUM_ELEMENT, 1);
		Deuterium->AddIsotope(Deuteron, 1);
	}
	log->info(2, "G4World: Deuterium element <", DEUTERIUM_ELEMENT, "> created with density <", d, ">");

	// Deuterium gas
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

	// Liquid Deuterium
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

	// Ammonia
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

	// helion isotope
	if (G4NistManager::Instance()->FindOrBuildElement(HELIUM3_ELEMENT) == nullptr) {
		Z           = 2;
		N           = 3;
		a           = 3.0160293 * CLHEP::g / CLHEP::mole;
		auto Helion = new G4Isotope(HELION_ISOTOPE, Z, N, a);

		// helium 3 element
		Helium3 = new G4Element(HELIUM3_ELEMENT, HELIUM3_ELEMENT, 1);
		Helium3->AddIsotope(Helion, 1);
	}
	log->info(2, "G4World: Helium 3 element <", HELIUM3_ELEMENT, "> created with density <", d, ">");

	// helium 3 material gas
	if (G4NistManager::Instance()->FindMaterial(HELIUM3GAS_MATERIAL) == nullptr) {
		// Density at 21.1°C (70°F) : 0.1650 kg/m3
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
	if (G4NistManager::Instance()->FindOrBuildElement(TRITIUM_ELEMENT) == nullptr) {
		// Tritium isotope
		Z           = 1;
		N           = 3;
		a           = 3.0160492 * CLHEP::g / CLHEP::mole;
		auto Triton = new G4Isotope(TRITON_ISOTOPE, Z, N, a);

		// Tritium element
		Tritium = new G4Element(TRITIUM_ELEMENT, TRITIUM_ELEMENT, 1);
		Tritium->AddIsotope(Triton, 1);
	}
	log->info(2, "G4World: Tritium element <", TRITIUM_ELEMENT, "> created with density <", d, ">");

	// Tritium gas
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

	//	// example how to get / print materials from G4
	//	G4MaterialTable* matTable = (G4MaterialTable*) G4Material::GetMaterialTable();
	//
	//
	//	for(unsigned i=0; i<matTable->size(); ++i)
	//	{
	//		G4Material* thisMat = (*(matTable))[i];
	//
	//		cout << " MATERIAL :" <<  thisMat->GetName()  << endl;
	//
	//
	//		for ( auto& [material, component]: thisMat->GetMatComponents() ) {
	//			cout << " name: " << material->GetName() << " " << component << endl;
	//		}
	//
	//		for ( auto element: *thisMat->GetElementVector() ) {
	//			cout << " name: " << element << endl;
	//		}
	//	}
}


void G4World::createG4SystemFactory(SystemMap*    gsystemsMap,
                                    const std::string& backup_material,
                                    int           check_overlaps) {
	// instantiating gSystemManager
	GManager manager(log, "G4World Manager");

	// Creating the native factory no matter what
	log->info(2, "G4World: registering default factory <", G4SYSTEMNATFACTORY, ">");


	manager.RegisterObjectFactory<G4NativeSystemFactory>(G4SYSTEMNATFACTORY);

	// registering factories in the manager
	// and adding them to g4systemFactory
	for (auto& [gsystemName, gsystem] : *gsystemsMap) {
		std::string factory   = gsystem->getFactoryName();
		std::string g4Factory = g4FactoryNameFromSystemFactory(factory);

		log->info(2, "G4World: creating factory <", g4Factory, "> to for system <", gsystemName, ">");

		// registering factories
		// this will always be false because the native factory is already registered
		if (factory == GSYSTEMASCIIFACTORYLABEL || factory == GSYSTEMSQLITETFACTORYLABEL ||
		    factory == GSYSTEMMYSQLTFACTORYLABEL) {
			// if factory not found, registering it in the manager and loading it into the map
			if (g4systemFactory.find(g4Factory) == g4systemFactory.end()) { manager.RegisterObjectFactory<G4NativeSystemFactory>(g4Factory); }
		}
		else if (factory == GSYSTEMCADTFACTORYLABEL) {
			// if factory not found, registering it in the manager and loading it into the map
			if (g4systemFactory.find(GSYSTEMCADTFACTORYLABEL) == g4systemFactory.end()) { manager.RegisterObjectFactory<G4CadSystemFactory>(g4Factory); }
		}

		// factories are registered, creating them
		if (g4systemFactory.find(g4Factory) == g4systemFactory.end()) {
			g4systemFactory[g4Factory] = manager.CreateObject<G4ObjectsFactory>(g4Factory);
			g4systemFactory[g4Factory]->initialize_context(log, check_overlaps, backup_material);
		}
	}

}

void G4World::buildMaterials(SystemMap* system_map) {
	// looping over gsystem in the gsystemsMap,
	// every GMaterial that is not built (due to dependencies) increments allRemainingMaterials
	std::vector<GMaterial*> thisIterationRemainingMaterials;
	unsigned long      allRemainingMaterials = 0;
	do {
		thisIterationRemainingMaterials.clear();

		for (const auto& [systemName, system] : *system_map) {
			// looping over getGVolumesMap in that system
			for (const auto& [gmaterialName, gmaterialPtr] : system->getGMaterialMap()) {
				if (createG4Material(gmaterialPtr) == false) { thisIterationRemainingMaterials.push_back(gmaterialPtr.get()); }
			}
		}

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
