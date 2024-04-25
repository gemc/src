// glibrary
#include "gfactory.h"

// g4system
#include "g4systemConventions.h"
#include "g4world.h"
#include "g4objectsFactories/g4objectsFactory.h"
#include "g4objectsFactories/g4native/g4NativeObjectsFactory.h"
#include "g4objectsFactories/cad/cadSystemFactory.h"

// geant4
#include "G4NistManager.hh"
using namespace CLHEP;

// c++
using namespace std;

G4World::G4World(GWorld *gworld, GOptions* gopts) {

	// instantiating volumes map
	g4volumesMap   = new map<string, G4Volume*>;
	g4materialsMap = new map<string, G4Material*>;

	// instantiating gSystemManager
	int verbosity = gopts->getInt(G4SYSTEMVERBOSITY);
	GManager g4SystemManager("G4World", verbosity);

	buildDefaultMaterialsElementsAndIsotopes(verbosity);

	map<string, G4ObjectsFactory*> g4systemFactory;

	// registering factories in the manager
	// and adding them to g4systemFactory
	for(auto &syst : *gworld->getSystemsMap()) {
		string factory = syst.second->getFactoryName();
		string g4Factory = g4FactoryNameFromSystemFactory(factory);

        // registering factories
		if(factory == GSYSTEMTEXTFACTORYLABEL || factory == GSYSTEMSQLITETFACTORYLABEL || factory == GSYSTEMMYSQLTFACTORYLABEL || factory == GSYSTEMJSONFACTORYLABEL) {
			// if factory not found, registering it in the manager and loading it into the map
			if(g4systemFactory.find(g4Factory) == g4systemFactory.end()) {
				g4SystemManager.RegisterObjectFactory<G4NativeSystemFactory>(g4Factory);
			}
		} else if(factory == GSYSTEMCADTFACTORYLABEL) {
			// if factory not found, registering it in the manager and loading it into the map
			if(g4systemFactory.find(GSYSTEMCADTFACTORYLABEL) == g4systemFactory.end()) {
				g4SystemManager.RegisterObjectFactory<G4CadSystemFactory>(g4Factory);
			}
		}

        // factories are registered, creating them
		if ( g4systemFactory.find(g4Factory) == g4systemFactory.end() ) {
			g4systemFactory[g4Factory] = g4SystemManager.CreateObject<G4ObjectsFactory>(g4Factory);
		}
	}

	// done with g4SystemManager
	g4SystemManager.clearDLMap();

	// now building geant4 objects

	// first, building all materials
	// looping over system in the gsystemsMap
	// every GMaterial that is not built (due to dependencies) increments allRemainingMaterials
	vector<GMaterial*> thisIterationRemainingMaterials;
	unsigned long allRemainingMaterials = 0;
	do {
		thisIterationRemainingMaterials.clear();

		for(auto [systemName, system] : *gworld->getSystemsMap()) {
			// looping over getGVolumesMap in that system
			for(auto [gmaterialName, gmaterial] : *system->getGMaterialMap() ) {
				if ( createG4Material(gmaterial, verbosity ) == false ) {
					thisIterationRemainingMaterials.push_back(gmaterial);
				}
			}
		}

		if(allRemainingMaterials != 0 && thisIterationRemainingMaterials.size() != 0) {
			if(allRemainingMaterials >= thisIterationRemainingMaterials.size()) {
				cerr << FATALERRORL << "dependencies are not being resolved: their number should diminish. Outstanding gmaterials:" << endl;
				for (auto &gmaterialLeft: thisIterationRemainingMaterials) {
					cerr << GTAB << "- <" << gmaterialLeft->getName() << ">" << endl;
				}
				gexit(EC__G4DEPENDENCIESNOTSOLVED);
			}
		} else {
			allRemainingMaterials = thisIterationRemainingMaterials.size();
		}

	} while (thisIterationRemainingMaterials.size() > 0);


	// every volume that is not built (due to dependencies) increments allRemainingVolumes
	vector<GVolume*> thisIterationRemainingVolumes;
	unsigned long allRemainingVolumes = 0;
	allRemainingVolumes = 0;
	do {
		thisIterationRemainingVolumes.clear();

		// looping over system in the gsystemsMap
		for(auto &system : *gworld->getSystemsMap()) {

			string systemName = system.first;
			string factory   = system.second->getFactoryName();

			string g4Factory = g4FactoryNameFromSystemFactory(factory);

			// looping over getGVolumesMap in that system
			for(auto& [volumeName, gvolume] : *system.second->getGVolumesMap() ) {
				if(g4systemFactory.find(g4Factory) != g4systemFactory.end()) {

					if(verbosity == GVERBOSITY_DETAILS) {
						G4cout << G4SYSTEMLOGHEADER << "using factory <" << KYEL << g4Factory << RST << ">, to build g4volume <" << KYEL << gvolume->getG4Name() << RST << ">" << endl;
					}

					// calling loadG4System
					// if a new system cannot be loaded, false is returned and the volumes added to thisIterationRemainingVolumes
					if(g4systemFactory[g4Factory]->loadG4System(gopts, gvolume, g4volumesMap) == false) {
                        if ( gvolume->getExistence() ) { thisIterationRemainingVolumes.push_back(gvolume); }
					}
				} else {
					G4cerr << FATALERRORL << "g4systemFactory factory <" << g4Factory << "> not found in map." << endl;
					gexit(EC__G4SYSTEMFACTORYNOTFOUND);
				}
			}
			if(verbosity == GVERBOSITY_DETAILS) {
				G4cout << G4SYSTEMLOGHEADER << system.first << " : " << thisIterationRemainingVolumes.size() << " remaining motherless g4volumes to be built: " <<  endl;
				for (auto &gvolumeLeft: thisIterationRemainingVolumes) {
					G4cout << GTAB << "- " << gvolumeLeft->getG4Name() << " with g4 mother <" << gvolumeLeft->getG4MotherName() << ">" << endl;
				}
			}
		}
		if(allRemainingVolumes != 0 && thisIterationRemainingVolumes.size() != 0) {
			if(allRemainingVolumes >= thisIterationRemainingVolumes.size()) {
				G4cerr << FATALERRORL << "dependencies are not being resolved: their number should diminish. Outstanding gvolumes:" << endl;
				for (auto &gvolumeLeft: thisIterationRemainingVolumes) {
					cerr << GTAB << "- <" << gvolumeLeft->getName() << "> with mother <" << gvolumeLeft->getG4MotherName() << ">" << endl;
				}
				gexit(EC__G4DEPENDENCIESNOTSOLVED);
			}
		} else {
			allRemainingVolumes = thisIterationRemainingVolumes.size();
		}
	} while (thisIterationRemainingVolumes.size() > 0);


	if ( gopts->getSwitch("logG4Materials") ) {
		G4NistManager::Instance()->ListMaterials("all");
	}

	if ( gopts->getSwitch("printSystemsMaterials") ) {
		cout << endl;
		G4MaterialTable* matTable = (G4MaterialTable*) G4Material::GetMaterialTable();
		for(unsigned i=0; i<matTable->size(); ++i) {

			G4Material* thisMat = (*(matTable))[i];

			cout << G4SYSTEMLOGHEADER << " GEMC Material: <" << KGRN << thisMat->GetName() << RST << ">, density: " << thisMat->GetDensity()/(g/cm3) << "g/cm3" << endl;

			for ( auto& [material, component]: thisMat->GetMatComponents() ) {
				cout << GTAB << " material " << material->GetName() << " fractional mass: " << component << endl;
			}

			auto nelements = int(thisMat->GetNumberOfElements());
			for ( int e=0; e<nelements; e++  ) {
				auto element = thisMat->GetElement(e);
				cout << GTAB << " element " << element->GetName() <<  " number of atoms: " << element->GetN() <<  endl;
			}

			cout << endl;
		}
		G4cout << *(G4Material::GetMaterialTable()) << G4endl;

	}

}


string G4World::g4FactoryNameFromSystemFactory(string factory) {
    if(factory == GSYSTEMTEXTFACTORYLABEL || factory == GSYSTEMSQLITETFACTORYLABEL || factory == GSYSTEMMYSQLTFACTORYLABEL || factory == GSYSTEMJSONFACTORYLABEL) {
		return G4SYSTEMNATFACTORY;
	} else if ( factory == GSYSTEMCADTFACTORYLABEL ) {
		return G4SYSTEMCADFACTORY;
	} else {
		cerr << FATALERRORL << "g4systemFactory factory <" << factory << "> not found in g4FactoryNameFromSystemFactory." << endl;
		gexit(EC__G4SYSTEMFACTORYNOTFOUND);
	}
	return UNINITIALIZEDSTRINGQUANTITY;
}


bool G4World::createG4Material(const GMaterial *gmaterial, int verbosity) {

	G4NistManager* NISTman = G4NistManager::Instance(); // material G4 Manager
	auto materialName  = gmaterial->getName();

	// only build the material if it's not found in geant4
	auto g4material =  NISTman->FindMaterial(materialName);
	if ( g4material != nullptr ) {
		if(verbosity == GVERBOSITY_DETAILS) {
			G4cout << G4SYSTEMLOGHEADER <<  materialName << " already exists in geant4. " << endl;
		}
		return true;
	}

	auto components = gmaterial->getComponents();
	auto amounts    = gmaterial->getAmounts();
	bool isChemical = gmaterial->isChemicalFormula();

	// scans material components
	// return false if any component does not exist yet
	for ( auto& componentName: components ) {
		if ( isChemical ) {
			if ( NISTman->FindOrBuildElement(componentName) == nullptr ) {
				if(verbosity == GVERBOSITY_SUMMARY) {
					G4cout << G4SYSTEMLOGHEADER << " element component " << componentName << " needed by material " << materialName << " not found yet " << endl;
				}
				return  false;
			}  else {
				if(verbosity == GVERBOSITY_SUMMARY) {
					G4cout << G4SYSTEMLOGHEADER << "material component " << componentName << " needed by material " << materialName << " now found" << endl;
				}
			}
		} else {
			if ( NISTman->FindOrBuildMaterial(componentName) == nullptr ) {
				if(verbosity == GVERBOSITY_SUMMARY) {
					G4cout << G4SYSTEMLOGHEADER << "material component " << componentName << " needed by material " << materialName << " not found yet " << endl;
				}
				return  false;
			} else {
				if(verbosity == GVERBOSITY_SUMMARY) {
					G4cout << G4SYSTEMLOGHEADER << "material component " << componentName << " needed by material " << materialName << " now found" << endl;
				}
			}
		}
	}

	// building material from components
	auto density = gmaterial->getDensity();
	(*g4materialsMap)[materialName] = new G4Material(materialName, density*g/cm3, (G4int) components.size());

	if ( isChemical ) {
		if(verbosity == GVERBOSITY_DETAILS) {
			G4cout << G4SYSTEMLOGHEADER << " Building material " << materialName << " with components: " << endl;
			for ( size_t i=0; i<components.size(); i++ ) {
				G4cout << GTAB << components[i] << ": " << amounts[i] << " atoms" << endl;
			}
		}
		for ( size_t i=0; i<components.size(); i++ ) {
			auto element = NISTman->FindOrBuildElement(components[i]);
			(*g4materialsMap)[materialName]->AddElement(element, (int) amounts[i]);
		}

	} else {
		if(verbosity == GVERBOSITY_DETAILS) {
			G4cout << G4SYSTEMLOGHEADER << " Building material " << materialName << " with components: " << endl;
			for ( size_t i=0; i<components.size(); i++ ) {
				G4cout << GTAB << components[i] << ": " << amounts[i] << " fractional mass" << endl;
			}
		}
		for ( size_t i=0; i<components.size(); i++ ) {
			auto material = NISTman->FindOrBuildMaterial(components[i]);
			(*g4materialsMap)[materialName]->AddMaterial(material, amounts[i]);
		}

	}


	return true;
}



void G4World::buildDefaultMaterialsElementsAndIsotopes([[maybe_unused]] int verbosity) {

	// isotopes not yet defined, defining them for the first time

	int Z, N;
	double a, d, T;


	// ----  Hydrogen

	// hydrogen gas
	Z = 1;
	a = 1.01*g/mole;
	d = 0.00275*g/cm3;
	T = 50.0*kelvin;
	G4Element* Hydrogen  = new G4Element(HYDROGEN_ELEMENT,  HYDROGEN_ELEMENT,  Z,  a);
	(*g4materialsMap)[HGAS_MATERIAL] = new G4Material(HGAS_MATERIAL,
																	  d,
																	  1,
																	  kStateGas,
																	  T);
	(*g4materialsMap)[HGAS_MATERIAL]->AddElement(Hydrogen, 1);

	// ----  Deuterium

	// Deuteron isotope
	Z = 1;
	N = 2;
	a = 2.0141018*g/mole;
	G4Isotope* Deuteron  = new G4Isotope(DEUTERON_ISOTOPE, Z, N, a);

	// Deuterium element
	G4Element* Deuterium = new G4Element(DEUTERIUM_ELEMENT, DEUTERIUM_ELEMENT, 1);
	Deuterium->AddIsotope(Deuteron, 1);


	// Deuterium gas
	d = 0.000452*g/cm3;
	T = 294.25*kelvin;
	(*g4materialsMap)[DEUTERIUMGAS_MATERIAL] = new G4Material(DEUTERIUMGAS_MATERIAL,
																				 d,
																				 1,
																				 kStateGas,
																				 T);
	(*g4materialsMap)[DEUTERIUMGAS_MATERIAL]->AddElement(Deuterium, 1);


	// Liquid Deuterium
	d = 0.169*g/cm3;
	T = 22.0*kelvin;
	(*g4materialsMap)[LD2_MATERIAL] = new G4Material(LD2_MATERIAL,
																	 d,
																	 1,
																	 kStateLiquid,
																	 T);
	(*g4materialsMap)[LD2_MATERIAL]->AddElement(Deuterium, 2);


	// Ammonia
	Z = 7;
	a = 14.01*g/mole;
	d = 1.007*g/cm3;
	T = 1.0*kelvin;
	G4Element* Nitrogen   = new G4Element(NITRO_ELEMENT, NITRO_ELEMENT,  Z,  a);
	(*g4materialsMap)[ND3_MATERIAL] = new G4Material(ND3_MATERIAL,
																	 d,
																	 2,
																	 kStateLiquid,
																	 T);
	(*g4materialsMap)[ND3_MATERIAL]->AddElement(Nitrogen, 1);
	(*g4materialsMap)[ND3_MATERIAL]->AddElement(Deuterium, 3);


	// ---- Helium 3


	// helion isotope
	Z = 2;
	N = 3;
	a = 3.0160293*g/mole;
	G4Isotope* Helion  = new G4Isotope(HELION_ISOTOPE, Z, N, a);

	// helium 3 element
	G4Element* Helium3 = new G4Element(HELIUM3_ELEMENT, HELIUM3_ELEMENT, 1);
	Helium3->AddIsotope(Helion, 1);

	// helium 3 material gas
	// Density at 21.1°C (70°F) : 0.1650 kg/m3
	d = 0.1650*mg/cm3;
	T = 294.25*kelvin;
	(*g4materialsMap)[HELIUM3GAS_MATERIAL] = new G4Material(HELIUM3GAS_MATERIAL,
																			  d,
																			  1,
																			  kStateGas,
																			  T);
	(*g4materialsMap)[HELIUM3GAS_MATERIAL]->AddElement(Helium3, 1);



	// ---- Tritium

	// Tritium isotope
	Z = 1;
	N = 3;
	a = 3.0160492*g/mole;
	G4Isotope* Triton  = new G4Isotope(TRITON_ISOTOPE, Z, N, a);

	// Tritium element
	G4Element* Tritium = new G4Element(TRITIUM_ELEMENT, TRITIUM_ELEMENT, 1);
	Tritium->AddIsotope(Triton, 1);

	// Tritium gas
	d = 0.0034*g/cm3;
	T = 40.0*kelvin;
	(*g4materialsMap)[TRITIUMGAS_MATERIAL] = new G4Material(TRITIUMGAS_MATERIAL,
																			  d,
																			  1,
																			  kStateGas, T);
	(*g4materialsMap)[TRITIUMGAS_MATERIAL]->AddElement(Tritium, 1);





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
