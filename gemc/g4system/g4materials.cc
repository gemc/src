#include "g4world.h"

// geant4
#include "G4Material.hh"
#include "G4Element.hh"
#include "G4Isotope.hh"
#include "gsystemConventions.h"

bool G4World::createG4Material(const std::shared_ptr<GMaterial> &gmaterial) {
	auto NISTman = G4NistManager::Instance(); // material G4 Manager
	auto materialName = gmaterial->getName();

	// Only build the material if it is not already available in Geant4.
	auto g4material = NISTman->FindMaterial(materialName);
	if (g4material != nullptr) {
		log->info(2, "Material <", materialName, "> already exists in G4NistManager");
		return true;
	}

	auto components = gmaterial->getComponents();
	auto amounts = gmaterial->getAmounts();
	bool isChemical = gmaterial->isChemicalFormula();

	// Scan material components:
	// return false if any component does not exist yet (caller will retry later).
	for (auto &componentName: components) {
		if (isChemical) {
			if (NISTman->FindOrBuildElement(componentName) == nullptr) {
				log->info(2, "Element <", componentName, ">, needed by ", materialName, ",  not found yet");
				return false;
			} else { log->info(2, "Element <", componentName, "> needed by ", materialName, " now found"); }
		} else {
			if (NISTman->FindOrBuildMaterial(componentName) == nullptr) {
				log->info(2, "Material <", componentName, ">, needed by ", materialName, ",  not found yet");
				return false;
			} else { log->info(2, "Material <", componentName, "> needed by ", materialName, " now found"); }
		}
	}

	// Build the composed material from its components.
	auto density = gmaterial->getDensity();
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
	} else {
		log->info(2, "Building material <", materialName, "> with components:");
		for (size_t i = 0; i < components.size(); i++) {
			log->info(2, "material <", components[i], "> with fractional mass: ", amounts[i]);
		}

		for (size_t i = 0; i < components.size(); i++) {
			auto material = NISTman->FindOrBuildMaterial(components[i]);
			g4materialsMap[materialName]->AddMaterial(material, amounts[i]);
		}
	}

	// optical properties
	auto photonEnergy = gmaterial->getPhotonEnergy();
	if (!photonEnergy.empty()) {
		auto *materialPropertiesTable = new G4MaterialPropertiesTable();
		bool hasOpticalProperties = false;

		auto addProperty = [&](const char *propertyName, const std::vector<double> &values) {
			if (values.empty()) { return; }
			if (values.size() != photonEnergy.size()) {
				log->error(ERR_GMATERIALOPTICALPROPERTYMISMATCH,
				           "material <", materialName, "> optical property <", propertyName, "> has ",
				           values.size(), " entries but photonEnergy has ", photonEnergy.size());
			}
			materialPropertiesTable->AddProperty(propertyName, photonEnergy, values);
			hasOpticalProperties = true;
		};

		auto addConstantProperty = [&](const char *propertyName, double value, bool isSet) {
			if (!isSet) { return; }
			materialPropertiesTable->AddConstProperty(propertyName, value);
			hasOpticalProperties = true;
		};

		// optical properties, these
		addProperty("RINDEX", gmaterial->getIndexOfRefraction());
		addProperty("ABSLENGTH", gmaterial->getAbsorptionLength());
		addProperty("REFLECTIVITY", gmaterial->getReflectivity());
		addProperty("EFFICIENCY", gmaterial->getEfficiency());
		// Geant4 11.x renamed the scintillation property keys (formerly FASTCOMPONENT etc.)
		addProperty("SCINTILLATIONCOMPONENT1", gmaterial->getFastComponent());
		addProperty("SCINTILLATIONCOMPONENT2", gmaterial->getSlowComponent());

		// scalar properties
		addConstantProperty("SCINTILLATIONYIELD", gmaterial->getScintillationYield(),
		                    gmaterial->hasScintillationYield());
		addConstantProperty("RESOLUTIONSCALE", gmaterial->getResolutionScale(),
		                    gmaterial->hasResolutionScale());

		addConstantProperty("SCINTILLATIONTIMECONSTANT1", gmaterial->getFasttimeConstant() * CLHEP::ns,
		                    gmaterial->hasFasttimeConstant());
		addConstantProperty("SCINTILLATIONTIMECONSTANT2", gmaterial->getSlowtimeConstant() * CLHEP::ns,
		                    gmaterial->hasSlowtimeConstant());
		addConstantProperty("SCINTILLATIONYIELD1", gmaterial->getYieldratio(),
		                    gmaterial->hasYieldratio());

		double getBirksConstant = gmaterial->getBirksConstant();
		if (gmaterial->hasBirksConstant()) {
			g4materialsMap[materialName]->GetIonisation()->SetBirksConstant(getBirksConstant);
			hasOpticalProperties = true;
		}

		// leaving RAYLEIGH last for consistency checks in
		addProperty("RAYLEIGH", gmaterial->getRayleigh());

		if (hasOpticalProperties) {
			g4materialsMap[materialName]->SetMaterialPropertiesTable(materialPropertiesTable);
			log->info(2, "Attached optical material properties table to material <", materialName, ">");
		} else { delete materialPropertiesTable; }
	}


	return true;
}

void G4World::buildDefaultMaterialsElementsAndIsotopes() {
	// Create a small set of commonly-used isotopes/elements/materials if they are missing.
	// These are defined using Geant4 primitives and then registered in the local map for reference.
	int Z, N;
	double a, d, T;

	// ----  Hydrogen

	// Hydrogen gas material definition (Hydrogen element + state/gas parameters).
	if (G4NistManager::Instance()->FindMaterial(HGAS_MATERIAL) == nullptr) {
		Z = 1;
		a = 1.01 * CLHEP::g / CLHEP::mole;
		d = 0.00275 * CLHEP::g / CLHEP::cm3;
		T = 50.0 * CLHEP::kelvin;
		auto Hydrogen = new G4Element(HYDROGEN_ELEMENT, HYDROGEN_ELEMENT, Z, a);
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
		Z = 1;
		N = 2;
		a = 2.0141018 * CLHEP::g / CLHEP::mole;
		auto Deuteron = new G4Isotope(DEUTERON_ISOTOPE, Z, N, a);

		// Deuterium element: isotope composition is explicitly set to the Deuteron isotope.
		Deuterium = new G4Element(DEUTERIUM_ELEMENT, DEUTERIUM_ELEMENT, 1);
		Deuterium->AddIsotope(Deuteron, 1);
	}
	log->info(2, "G4World: Deuterium element <", DEUTERIUM_ELEMENT, "> created with density <", d, ">");

	// Deuterium gas material.
	if (G4NistManager::Instance()->FindMaterial(DEUTERIUMGAS_MATERIAL) == nullptr) {
		d = 0.000452 * CLHEP::g / CLHEP::cm3;
		T = 294.25 * CLHEP::kelvin;
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
		d = 0.169 * CLHEP::g / CLHEP::cm3;
		T = 22.0 * CLHEP::kelvin;
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
		Z = 7;
		a = 14.01 * CLHEP::g / CLHEP::mole;
		d = 1.007 * CLHEP::g / CLHEP::cm3;
		T = 1.0 * CLHEP::kelvin;
		auto Nitrogen = new G4Element(NITRO_ELEMENT, NITRO_ELEMENT, Z, a);
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
		Z = 2;
		N = 3;
		a = 3.0160293 * CLHEP::g / CLHEP::mole;
		auto Helion = new G4Isotope(HELION_ISOTOPE, Z, N, a);

		// Helium-3 element: isotope composition is explicitly set to the Helion isotope.
		Helium3 = new G4Element(HELIUM3_ELEMENT, HELIUM3_ELEMENT, 1);
		Helium3->AddIsotope(Helion, 1);
	}
	log->info(2, "G4World: Helium 3 element <", HELIUM3_ELEMENT, "> created with density <", d, ">");

	// Helium-3 gas material definition.
	if (G4NistManager::Instance()->FindMaterial(HELIUM3GAS_MATERIAL) == nullptr) {
		// Density at 21.1°C (70°F): 0.1650 kg/m3.
		d = 0.1650 * CLHEP::mg / CLHEP::cm3;
		T = 294.25 * CLHEP::kelvin;
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
		Z = 1;
		N = 3;
		a = 3.0160492 * CLHEP::g / CLHEP::mole;
		auto Triton = new G4Isotope(TRITON_ISOTOPE, Z, N, a);

		Tritium = new G4Element(TRITIUM_ELEMENT, TRITIUM_ELEMENT, 1);
		Tritium->AddIsotope(Triton, 1);
	}
	log->info(2, "G4World: Tritium element <", TRITIUM_ELEMENT, "> created with density <", d, ">");

	// Tritium gas material definition.
	if (G4NistManager::Instance()->FindMaterial(TRITIUMGAS_MATERIAL) == nullptr) {
		d = 0.0034 * CLHEP::g / CLHEP::cm3;
		T = 40.0 * CLHEP::kelvin;
		g4materialsMap[TRITIUMGAS_MATERIAL] = new G4Material(TRITIUMGAS_MATERIAL,
		                                                     d,
		                                                     1,
		                                                     kStateGas, T);
		g4materialsMap[TRITIUMGAS_MATERIAL]->AddElement(Tritium, 1);
	}
	log->info(2, "G4World: Tritium gas material <", TRITIUMGAS_MATERIAL, "> created with density <", d, ">");
}
