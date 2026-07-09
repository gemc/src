// gdetectorConstruction
#include "gdetectorConstruction.h"
#include "gdetector_options.h"

// gemc
#include "gdynamicdigitizationConventions.h"
#include "gtouchableConventions.h"
#include "gsystemConventions.h"
#include "gsystem_options.h"
#include "gDosimeterDigitization.h"
#include "gFluxDigitization.h"
#include "gPhotonDetectorDigitization.h"
#include "gParticleCounterDigitization.h"
#include "gfield_options.h"

// geant4
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4Threading.hh"
#include "G4UserLimits.hh"
#include "G4VVisManager.hh"

namespace {
bool is_unset_field_name(const std::string& name) {
	return name.empty() || name == UNINITIALIZEDSTRINGQUANTITY || name == "NULL" || name == "not provided";
}

class GVisManagerGuard : public G4VVisManager {
public:
	static void set(G4VVisManager* visManager) { SetConcreteInstance(visManager); }

	void Draw(const G4Circle&, const G4Transform3D&) override {}
	void Draw(const G4Polyhedron&, const G4Transform3D&) override {}
	void Draw(const G4Polyline&, const G4Transform3D&) override {}
	void Draw(const G4Polymarker&, const G4Transform3D&) override {}
	void Draw(const G4Square&, const G4Transform3D&) override {}
	void Draw(const G4Text&, const G4Transform3D&) override {}
	void Draw2D(const G4Circle&, const G4Transform3D&) override {}
	void Draw2D(const G4Polyhedron&, const G4Transform3D&) override {}
	void Draw2D(const G4Polyline&, const G4Transform3D&) override {}
	void Draw2D(const G4Polymarker&, const G4Transform3D&) override {}
	void Draw2D(const G4Square&, const G4Transform3D&) override {}
	void Draw2D(const G4Text&, const G4Transform3D&) override {}
	void Draw(const G4VTrajectory&) override {}
	void Draw(const G4VHit&) override {}
	void Draw(const G4VDigi&) override {}
	void Draw(const G4LogicalVolume&, const G4VisAttributes&, const G4Transform3D&) override {}
	void Draw(const G4VPhysicalVolume&, const G4VisAttributes&, const G4Transform3D&) override {}
	void Draw(const G4VSolid&, const G4VisAttributes&, const G4Transform3D&) override {}
	void BeginDraw(const G4Transform3D&) override {}
	void EndDraw() override {}
	void BeginDraw2D(const G4Transform3D&) override {}
	void EndDraw2D() override {}
	void GeometryHasChanged() override {}
	void DispatchToModel(const G4VTrajectory&) override {}
	G4bool FilterTrajectory(const G4VTrajectory&) override { return true; }
	G4bool FilterHit(const G4VHit&) override { return true; }
	G4bool FilterDigi(const G4VDigi&) override { return true; }
};
}

G4ThreadLocal GMagneto *GDetectorConstruction::gmagneto = nullptr;
G4ThreadLocal std::map<std::string, GSensitiveDetector*>* GDetectorConstruction::tlSDMap = nullptr;

GDetectorConstruction::GDetectorConstruction(std::shared_ptr<GOptions> gopts)
	: GBase(gopts, GDETECTOR_LOGGER),
	  G4VUserDetectorConstruction(), // Geant4 base class.
	  gopt(gopts) {
	// Map is populated after SDs exist, in the SD/field construction path.
	digitization_routines_map = std::make_shared<gdynamicdigitization::dRoutinesMap>();
}

// Builds (or rebuilds) the GEMC world and then the Geant4 world.
G4VPhysicalVolume *GDetectorConstruction::Construct() {
	log->debug(NORMAL, FUNCTION_NAME);

	// Delete old geometry objects if they exist.
	// These shared_ptr resets guarantee we won't keep references to stale world objects.
	gworld.reset();
	g4world.reset();

	// - if no systems are provided, we just launched gemc: create from options
	// - otherwise, it's a geometry re-load. use existing systems.
	if (gsystems.empty()) {
		log->debug(NORMAL, FUNCTION_NAME, "creating world from options");
		gworld = std::make_shared<GWorld>(gopt);
	} else {
		log->debug(NORMAL, FUNCTION_NAME, "creating world from a gsystem vector of size ", gsystems.size());
		gworld = std::make_shared<GWorld>(gopt, cloneSystemDescriptors(gsystems));
	}

	// Build Geant4 world (solids, logical and physical volumes) based on the GEMC world.
	g4world = std::make_shared<G4World>(gworld.get(), gopt);

	auto nsdetectors = gworld->getSensitiveDetectorsList().size();

	// tally with number :
	log->info(0, "Tally summary: \n - ", gworld->get_number_of_volumes() - 1, " volumes\n - ",
	          g4world->number_of_volumes(), " geant4 built volumes\n - ",
	          nsdetectors, " sensitive detectors\n");


	// Return the physical volume for the ROOT world volume.
	return g4world->getG4Volume(ROOTWORLDGVOLUMENAME)->getPhysical();
}

// Installs sensitive detectors and EM fields for the constructed geometry.
void GDetectorConstruction::ConstructSDandField() {
	auto sdManager = G4SDManager::GetSDMpointer();

	log->debug(NORMAL, FUNCTION_NAME);

	// Deactivate all SDs this thread registered in prior geometry loads.
	// G4SDManager retains SDs indefinitely across reloads; an active stale SD has
	// Initialize() called at the start of every event even when no volume uses it,
	// producing orphan hit collections whose names are absent from the digitization map.
	// We deactivate them here and reactivate only those needed for the current geometry.
	if (!tlSDMap) {
		tlSDMap = new std::map<std::string, GSensitiveDetector*>();
	}
	for (auto& [name, sd] : *tlSDMap) {
		sd->Activate(false);
	}

	// Local cache of sensitive detectors keyed by digitization name.
	// Multiple volumes can share the same digitization name and therefore reuse one SD instance.
	std::unordered_map<std::string, GSensitiveDetector *> sensitiveDetectorsMap;

	// --- Electromagnetic field reset (-no_field) --------------------------------------------------
	// Parse -no_field: a gvolume name, a whitespace/comma-separated list of gvolume names, or 'all'.
	// 'all' resets every per-volume field and the global field; a name resets only that volume's field.
	bool                  disable_all_fields = false;
	std::set<std::string> no_field_volumes;
	{
		auto no_field_value = gopt->getScalarString(NO_FIELD_OPTION);
		if (no_field_value != "" && no_field_value != UNINITIALIZEDSTRINGQUANTITY && no_field_value != "NULL") {
			if (no_field_value == NO_FIELD_ALL) { disable_all_fields = true; }
			else {
				for (auto &c : no_field_value) { if (c == ',') { c = ' '; } }
				for (const auto &v : gutilities::getStringVectorFromString(no_field_value)) {
					no_field_volumes.insert(v);
				}
			}
		}
	}

	// Global field name, honoring -no_field=all.
	const bool no_system_selected = gsystems.empty() && gsystem::getSystems(gopt).empty();
	auto global_field_name = gopt->getScalarString(GLOBAL_FIELD_OPTION);
	if (no_system_selected && !disable_all_fields && is_unset_field_name(global_field_name)) {
		std::vector<std::string> configured_fields;
		for (const auto& field_definition : gfields::get_GFieldDefinition(gopt)) {
			if (!is_unset_field_name(field_definition.name) && field_definition.name != goptions::NODFLT) {
				configured_fields.push_back(field_definition.name);
			}
		}
		if (configured_fields.size() == 1) {
			global_field_name = configured_fields.front();
			log->info(1, "No explicit global field selected: using the only configured field <",
			          global_field_name, "> on the ROOT world volume.");
		}
	}
	const bool global_field_set = !disable_all_fields && !is_unset_field_name(global_field_name);

	// First pass: collect the fields that are actually used so only their plugins and maps are loaded.
	// Volumes (or the global field) reset via -no_field are excluded here and never trigger a load.
	std::set<std::string> required_fields;
	std::set<std::string> matched_no_field;
	if (!disable_all_fields) {
		for (const auto &[systemName, gsystemPtr]: *gworld->getSystemsMap()) {
			for (const auto &[volumeName, gvolumePtr]: gsystemPtr->getGVolumesMap()) {
				const auto &g4name = gvolumePtr->getG4Name();
				const bool  reset  = no_field_volumes.count(volumeName) || no_field_volumes.count(g4name);
				if (no_field_volumes.count(volumeName)) { matched_no_field.insert(volumeName); }
				if (no_field_volumes.count(g4name)) { matched_no_field.insert(g4name); }

				const auto &field_name = gvolumePtr->getEMField();
				if (is_unset_field_name(field_name) || reset) { continue; }
				required_fields.insert(field_name);
			}
		}
		for (const auto &name: no_field_volumes) {
			if (!matched_no_field.count(name)) {
				log->warning("-", NO_FIELD_OPTION, ": volume <", name, "> not found in the geometry.");
			}
		}
	}
	if (global_field_set) { required_fields.insert(global_field_name); }

	// Build the magnetic-field registry only when at least one field is needed: if every field was reset
	// (e.g. -no_field=all) no plugin or field map is loaded at all.
	if (!required_fields.empty() && gmagneto == nullptr) { gmagneto = new GMagneto(gopt, required_fields); }

	// Loop over all systems and their volumes.
	for (const auto &[systemName, gsystemPtr]: *gworld->getSystemsMap()) {
		for (const auto &[volumeName, gvolumePtr]: gsystemPtr->getGVolumesMap()) {
			auto const &digitizationName = gvolumePtr->getDigitization();
			auto const &g4name = gvolumePtr->getG4Name();

			// Component volumes are boolean-operation building blocks: they have a
			// solid but no logical volume, no field, and no sensitivity.
			if (gvolumePtr->getMaterial() == GSYSTEMCOMPONENTMATERIAL) { continue; }

			auto *g4volume = g4world->getG4Volume(g4name)->getLogical();

			// Ensure the Geant4 logical volume exists.
			// Some GEMC volumes can be "copy-of" another volume; in that case, reuse the
			// referenced Geant4 logical volume rather than failing.
			if (g4volume == nullptr) {
				std::string copyOf = gvolumePtr->getCopyOf();
				if (copyOf != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) {
					auto gsystem = gvolumePtr->getSystem();
					auto volume_copy = gsystem + "/" + copyOf;
					auto copyG4Volume = g4world->getG4Volume(volume_copy)->getLogical();
					if (copyG4Volume != nullptr) { g4volume = copyG4Volume; } else {
						log->error(ERR_GVOLUMENOTFOUND, FUNCTION_NAME,
						           " Logical volume copy <" + volume_copy + "> not found.");
					}
				}
			}
			if (g4volume == nullptr) {
				log->error(ERR_GVOLUMENOTFOUND, FUNCTION_NAME, " Logical volume <" + g4name + "> not found.");
			}

			// Skip volumes with no digitization.
			if (digitizationName != "" && digitizationName != UNINITIALIZEDSTRINGQUANTITY) {
				// Obtain (or create) the sensitive detector for this digitization name.
				// We reuse an existing SD object already in G4SDManager rather than creating a
				// new one because AddNewDetector() keeps the OLD object on duplicate names (DET1010).
				// If we created a new SD, G4SDManager would call Initialize() on the stale object
				// while SetSensitiveDetector() pointed the logical volume to the new one — leaving
				// the new SD's gHitsCollection uninitialized when ProcessHits() is called.
				if (sensitiveDetectorsMap.find(digitizationName) == sensitiveDetectorsMap.end()) {
					// Reuse a previously registered SD for this name if one exists on this thread.
					// AddNewDetector() silently keeps the OLD object on duplicate names (DET1010);
					// reusing the same pointer avoids that and ensures G4SDManager's Initialize()
					// fires on the same object that SetSensitiveDetector() wires to the volumes.
					auto tlIt = tlSDMap->find(digitizationName);
					if (tlIt != tlSDMap->end()) {
						log->info(2, "Reusing existing sensitive detector <", digitizationName, "> for volume <", g4name, ">");
						tlIt->second->resetTouchableMap();
						tlIt->second->Activate(true);
						sensitiveDetectorsMap[digitizationName] = tlIt->second;
					} else {
						log->info(2, "Creating new sensitive detector <", digitizationName, "> for volume <", g4name, ">");
						auto* newSD = new GSensitiveDetector(digitizationName, gopt);
						sdManager->AddNewDetector(newSD);
						sensitiveDetectorsMap[digitizationName] = newSD;
						(*tlSDMap)[digitizationName] = newSD;
					}
				} else {
					log->info(2, "Sensitive detector <", digitizationName,
					          "> is already created and available for volume <", g4name, ">");
				}

				// Register the volume touchable with the sensitive detector.
				// The touchable encodes identity and dimension metadata needed by digitization.
				const auto &vdimensions = gvolumePtr->getDetectorDimensions();
				const auto &identity = gvolumePtr->getGIdentity();
				const auto &mass = g4volume->GetMass();
				auto this_gtouchable = std::make_shared<
					GTouchable>(gopt, digitizationName, identity, vdimensions, mass);
				sensitiveDetectorsMap[digitizationName]->registerGVolumeTouchable(g4name, this_gtouchable);

				// Attach the SD to the logical volume (no AddNewDetector call needed for reused SDs).
				g4volume->SetSensitiveDetector(sensitiveDetectorsMap[digitizationName]);

				//	auto maxStep =

				//g4volume->SetUserLimits(new G4UserLimits(0.1*mm, 0.1*mm));

				log->info(2, "Logical Volume  <" + g4name + "> has been successfully assigned to SD.",
				          sensitiveDetectorsMap[digitizationName]);
			}

			// Process electromagnetic fields.
			// If a volume declares an EM field, install a per-volume field manager configured by the
			// named field map, unless that field was reset via -no_field (a matching name, or =all).
			const auto &field_name           = gvolumePtr->getEMField();
			const bool  volume_field_present = !is_unset_field_name(field_name);
			const bool  volume_field_reset   = disable_all_fields ||
				no_field_volumes.count(volumeName) || no_field_volumes.count(g4name);
			if (volume_field_present && volume_field_reset) {
				log->info(2, "Volume <", volumeName, "> field <", field_name, "> reset by -",
				          NO_FIELD_OPTION, ": no field installed.");
			} else if (volume_field_present) {
				log->info(2, "Volume <", volumeName, "> has field: <", field_name,
				          ">. Looking into field map definitions.");
				log->info(2, "Setting field manager for volume <", g4name, "> with field <", field_name, ">");
				g4world->setFieldManagerForVolume(g4name, gmagneto->getFieldMgr(field_name).get(), true);
			}
		}
	}

	// Process the global field, if requested and not reset via -no_field=all.
	// A global field is associated with the ROOT world volume and propagated to all daughters, so it
	// applies everywhere a more specific per-volume field has not been installed.
	if (global_field_set) {
		log->info(2, "Setting global field manager for the ROOT world volume <", ROOTWORLDGVOLUMENAME,
		          "> with field <", global_field_name, ">");
		g4world->setFieldManagerForVolume(ROOTWORLDGVOLUMENAME,
		                                  gmagneto->getFieldMgr(global_field_name).get(), true);
	} else if (disable_all_fields && global_field_name != "" &&
		!is_unset_field_name(global_field_name)) {
		log->info(2, "Global field <", global_field_name, "> reset by -", NO_FIELD_OPTION, "=",
		          NO_FIELD_ALL, ": none installed.");
	}

	// Load digitization plugins only when geometry has changed and only on the master thread.
	// digiplugins_need_reload is set true by reload_geometry() and prepare_geometry_for_run()
	// so that routine BeamOn re-initializations (which also call ConstructSDandField() on the
	// master) do not clear the shared map while worker threads may be concurrently reading it.
	if (G4Threading::IsMasterThread() && digiplugins_need_reload) {
		loadDigitizationPlugins();
		digiplugins_need_reload = false;
	}

	// Bind each digitization routine to its corresponding sensitive detector.
	const auto sdetectors = gworld->getSensitiveDetectorsList();
	for (auto &sdname: sdetectors) {
		auto digitization_routine = digitization_routines_map->at(sdname);
		double maxStep = digitization_routine->readoutSpecs->getMaxStep();

		sensitiveDetectorsMap[sdname]->assign_digi_routine(digitization_routine);
		log->info(1, "Digitization routine <" + sdname + "> has been successfully assigned to SD.",
		          sensitiveDetectorsMap[sdname]);

		// Loop over all systems and their volumes.
		// and assign max step to the corresponding logical volume
		for (const auto &[systemName, gsystemPtr]: *gworld->getSystemsMap()) {
			for (const auto &[volumeName, gvolumePtr]: gsystemPtr->getGVolumesMap()) {
				auto const &digitizationName = gvolumePtr->getDigitization();
				if (digitizationName == sdname) {
					auto const &g4name = gvolumePtr->getG4Name();
					auto *g4volume = g4world->getG4Volume(g4name)->getLogical();

					// g4volume->SetUserLimits(new G4UserLimits(maxStep, maxStep)); // this will also kill track cause
					// the second argument is max track length
					g4volume->SetUserLimits(new G4UserLimits(maxStep));

					log->info(1, "Setting G4UserLimits for volume <", g4name, "> with maxStep <", maxStep, ">");
				}
			}
		}
	}
}

// Loads (or dynamically resolves) the digitization routine for each sensitive detector.
void GDetectorConstruction::loadDigitizationPlugins() {
	// Clear stale entries so a reloaded geometry always gets fresh routines.
	// emplace() would silently skip existing keys, causing hits not to be recorded
	// when the geometry is reloaded with the same SD names.
	digitization_routines_map->clear();

	// Resolve the variation each routine uses to load constants / translation tables.
	// By default a routine follows the variation of the gsystem it belongs to; the
	// digitization_variation option, when set, overrides that for every routine.
	const std::string digiVariationOverride = gopt->getScalarString("digitization_variation");
	const bool        overrideVariation     = (digiVariationOverride != UNINITIALIZEDSTRINGQUANTITY);

	// Map each digitization (sensitive-detector) name to its system's variation.
	std::map<std::string, std::string> systemVariationFor;
	for (const auto& [systemName, gsystemPtr] : *gworld->getSystemsMap()) {
		for (const auto& [volumeName, gvolumePtr] : gsystemPtr->getGVolumesMap()) {
			const auto& digiName = gvolumePtr->getDigitization();
			if (digiName != "" && digiName != UNINITIALIZEDSTRINGQUANTITY) {
				systemVariationFor.emplace(digiName, gsystemPtr->getVariation());
			}
		}
	}

	const auto sdetectors = gworld->getSensitiveDetectorsList();

	for (auto &sdname: sdetectors) {
		if (sdname == FLUXNAME) {
			log->info(1, "Loading flux digitization plugin for routine <" + sdname + ">");
			digitization_routines_map->emplace(sdname, std::make_shared<GFluxDigitization>(gopt));
		} else if (sdname == GPHOTON_DETECTORNAME) {
			log->info(1, "Loading gPhotonDetector digitization plugin for routine <" + sdname + ">");
			digitization_routines_map->emplace(sdname, std::make_shared<GPhotonDetectorDigitization>(gopt));
		} else if (sdname == COUNTERNAME) {
			log->info(1, "Loading particle counter digitization plugin for routine <" + sdname + ">");
			digitization_routines_map->emplace(sdname, std::make_shared<GParticleCounterDigitization>(gopt));
		} else if (sdname == DOSIMETERNAME) {
			log->info(1, "Loading dosimeter digitization plugin for routine <" + sdname + ">");
			digitization_routines_map->emplace(sdname, std::make_shared<GDosimeterDigitization>(gopt));
		} else {
			// if it's not in the map already, add it
			log->info(0, "Loading new digitization plugin for routine <" + sdname + ">");
			digitization_routines_map->emplace(sdname, gdynamicdigitization::load_dynamicRoutine(sdname, gopt));
		}

		// Ensure each routine uses the correct logger and is configured for readout.
		digitization_routines_map->at(sdname)->set_loggers(gopt);

		// Resolve and store the variation used when loading this routine's constants/TT:
		// the digitization_variation option when set, otherwise the routine's gsystem variation.
		std::string variation = "default";
		if (const auto it = systemVariationFor.find(sdname); it != systemVariationFor.end()) {
			variation = it->second;
		}
		if (overrideVariation) { variation = digiVariationOverride; }
		digitization_routines_map->at(sdname)->setDigitizationVariation(variation);

		// Resolve whether this system applies threshold / efficiency rejection, from the
		// global applyThresholds / applyInefficiencies options (default: neither).
		digitization_routines_map->at(sdname)->setHitRejectionPolicies(sdname);

		if (digitization_routines_map->at(sdname)->defineReadoutSpecs()) {
			log->info(1, "Digitization routine <" + sdname + "> has been successfully defined.");
		} else { log->error(ERR_DEFINESPECFAIL, "defineReadoutSpecs failure for <" + sdname + ">"); }
	}
}


SystemList GDetectorConstruction::cloneSystemDescriptors(const SystemList& systems) const {
	SystemList descriptors;
	descriptors.reserve(systems.size());

	for (const auto& system: systems) {
		if (system != nullptr) {
			descriptors.emplace_back(system->descriptorClone(gopt));
		}
	}

	return descriptors;
}


void GDetectorConstruction::reload_geometry(SystemList sl) {
	// Geometry is changing: ensure loadDigitizationPlugins() runs on the next ConstructSDandField().
	digiplugins_need_reload = true;

	// it could be empty for tests
	if (!sl.empty()) {
		gsystems = cloneSystemDescriptors(sl);
	}

	// Reconstruct the master geometry immediately so GUI pages can inspect the
	// new world without starting worker threads during a setup-tab reload.
	auto rm = G4RunManager::GetRunManager();

	if (rm) {
		// Null out the vis manager while we clean and rebuild the geometry stores.
		// G4*Store::Clean() would otherwise leave the ToolsSG scene graph with
		// dangling references to deleted Geant4 objects, causing a crash on the
		// next visualization flush.
		auto* visManager = G4VVisManager::GetConcreteInstance();
		GVisManagerGuard::set(nullptr);
		rm->DefineWorldVolume(Construct());
		GVisManagerGuard::set(visManager);
		ConstructSDandField();
	} else { log->error(1, "GDetectorConstruction::reload_geometry", "Geant4 Run manager not found."); }
}

void GDetectorConstruction::prepare_geometry_for_run() {
	auto rm = G4RunManager::GetRunManager();

	if (rm) {
		auto* visManager = G4VVisManager::GetConcreteInstance();
		GVisManagerGuard::set(nullptr);
		// Geometry is being rebuilt: ensure loadDigitizationPlugins() runs inside Initialize().
		digiplugins_need_reload = true;
		rm->ReinitializeGeometry(true, true);
		rm->GeometryHasBeenModified();
		// Optical processes such as G4Cerenkov cache material-property tables by
		// material index. A setup-tab reload can introduce a different variation's
		// optical material, so force physics tables to rebuild after the new
		// geometry/material set has been installed.
		rm->PhysicsHasBeenModified();
		rm->Initialize();
		GVisManagerGuard::set(visManager);
	} else { log->error(1, "GDetectorConstruction::prepare_geometry_for_run", "Geant4 Run manager not found."); }
}
