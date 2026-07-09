// g4mirrors.cc : optical surface (mirror) construction.
/**
 * \file   g4mirrors.cc
 * @ingroup g4system_geometry
 * \brief  Implementation of G4World optical surface construction (phase 5).
 *
 * @details
 * Header documentation in \c g4world.h is authoritative. This file implements:
 * - table-driven mapping of surface type/finish/model strings to Geant4 enums
 *   (unknown values are hard errors, unlike GEMC2 which silently ignored them);
 * - one shared \c G4OpticalSurface per mirror definition, reused by every volume
 *   that references it (GEMC2 created one surface and one properties table per volume);
 * - skin and border logical surface attachment.
 */

// gemc
#include "g4world.h"
#include "gsystemConventions.h"
#include "g4systemConventions.h"

// geant4
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"

namespace {

const std::unordered_map<std::string, G4SurfaceType>& surfaceTypesMap() {
	static const std::unordered_map<std::string, G4SurfaceType> types = {
		{"dielectric_metal", dielectric_metal},
		{"dielectric_dielectric", dielectric_dielectric},
		{"dielectric_LUT", dielectric_LUT},
		{"dielectric_LUTDAVIS", dielectric_LUTDAVIS},
		{"dielectric_dichroic", dielectric_dichroic},
		{"firsov", firsov},
		{"x_ray", x_ray}
	};
	return types;
}

const std::unordered_map<std::string, G4OpticalSurfaceFinish>& surfaceFinishesMap() {
	static const std::unordered_map<std::string, G4OpticalSurfaceFinish> finishes = {
		{"polished", polished},
		{"polishedfrontpainted", polishedfrontpainted},
		{"polishedbackpainted", polishedbackpainted},
		{"ground", ground},
		{"groundfrontpainted", groundfrontpainted},
		{"groundbackpainted", groundbackpainted},
		// LBNL LUT model finishes
		{"polishedlumirrorair", polishedlumirrorair},
		{"polishedlumirrorglue", polishedlumirrorglue},
		{"polishedair", polishedair},
		{"polishedteflonair", polishedteflonair},
		{"polishedtioair", polishedtioair},
		{"polishedtyvekair", polishedtyvekair},
		{"polishedvm2000air", polishedvm2000air},
		{"polishedvm2000glue", polishedvm2000glue},
		{"etchedlumirrorair", etchedlumirrorair},
		{"etchedlumirrorglue", etchedlumirrorglue},
		{"etchedair", etchedair},
		{"etchedteflonair", etchedteflonair},
		{"etchedtioair", etchedtioair},
		{"etchedtyvekair", etchedtyvekair},
		{"etchedvm2000air", etchedvm2000air},
		{"etchedvm2000glue", etchedvm2000glue},
		{"groundlumirrorair", groundlumirrorair},
		{"groundlumirrorglue", groundlumirrorglue},
		{"groundair", groundair},
		{"groundteflonair", groundteflonair},
		{"groundtioair", groundtioair},
		{"groundtyvekair", groundtyvekair},
		{"groundvm2000air", groundvm2000air},
		{"groundvm2000glue", groundvm2000glue},
		// DAVIS model finishes
		{"Rough_LUT", Rough_LUT},
		{"RoughTeflon_LUT", RoughTeflon_LUT},
		{"RoughESR_LUT", RoughESR_LUT},
		{"RoughESRGrease_LUT", RoughESRGrease_LUT},
		{"Polished_LUT", Polished_LUT},
		{"PolishedTeflon_LUT", PolishedTeflon_LUT},
		{"PolishedESR_LUT", PolishedESR_LUT},
		{"PolishedESRGrease_LUT", PolishedESRGrease_LUT},
		{"Detector_LUT", Detector_LUT}
	};
	return finishes;
}

const std::unordered_map<std::string, G4OpticalSurfaceModel>& surfaceModelsMap() {
	static const std::unordered_map<std::string, G4OpticalSurfaceModel> models = {
		{"glisur", glisur},
		{"unified", unified},
		{"LUT", LUT},
		{"DAVIS", DAVIS},
		{"dichroic", dichroic}
	};
	return models;
}

} // namespace


void G4World::buildOpticalSurfaces(SystemMap* system_map) {
	int nSkinSurfaces   = 0;
	int nBorderSurfaces = 0;

	for (auto& [systemName, gsystem] : *system_map) {
		for (auto& [volumeName, gvolumePtr] : gsystem->getGVolumesMap()) {
			auto* gvolume = gvolumePtr.get();

			std::string mirrorName = gvolume->getMirror();
			if (gutilities::is_unset(mirrorName) || mirrorName == "no" || mirrorName == "none") { continue; }

			// Nonexistent volumes are skipped quietly, consistent with the volume build phase.
			if (!gvolume->getExistence()) { continue; }

			const GMirror* gmirror = gsystem->getGMirror(mirrorName);
			if (gmirror == nullptr) {
				log->error(ERR_G4MIRRORNOTFOUND, "mirror <", mirrorName, "> referenced by volume <",
				           volumeName, "> is not defined in system <", systemName, ">");
			}

			G4OpticalSurface* surface = getOrCreateOpticalSurface(systemName, gmirror);

			const G4Volume* g4volume = getG4Volume(gvolume->getG4Name());
			if (g4volume == nullptr) {
				log->error(ERR_G4VOLUMEBUILDFAILED, "volume <", gvolume->getG4Name(),
				           "> referencing mirror <", mirrorName, "> was not built");
			}

			if (gmirror->isSkinSurface()) {
				// The skin surface covers the entire logical volume boundary.
				new G4LogicalSkinSurface(gvolume->getG4Name(), g4volume->getLogical(), surface);
				nSkinSurfaces++;
				log->info(2, "G4World: skin surface <", systemName, GSYSTEM_DELIMITER, mirrorName,
				          "> attached to volume <", gvolume->getG4Name(), ">");
			}
			else {
				// The border surface applies to photons crossing from this volume into the
				// border volume, which must belong to the same system.
				std::string     borderG4Name   = systemName + GSYSTEM_DELIMITER + gmirror->getBorder();
				const G4Volume* borderG4Volume = getG4Volume(borderG4Name);
				if (borderG4Volume == nullptr || borderG4Volume->getPhysical() == nullptr) {
					log->error(ERR_G4MIRRORNOTFOUND, "border volume <", borderG4Name,
					           "> of mirror <", mirrorName, "> referenced by volume <",
					           volumeName, "> is not built");
				}
				std::string surfaceName = gvolume->getG4Name() + "_to_" + borderG4Name;
				new G4LogicalBorderSurface(surfaceName, g4volume->getPhysical(),
				                           borderG4Volume->getPhysical(), surface);
				nBorderSurfaces++;
				log->info(2, "G4World: border surface <", surfaceName, "> using mirror <",
				          systemName, GSYSTEM_DELIMITER, mirrorName, ">");
			}
		}
	}

	if (nSkinSurfaces + nBorderSurfaces > 0) {
		log->info(1, "G4World: created ", g4opticalSurfacesMap.size(), " optical surfaces, attached to ",
		          nSkinSurfaces, " skin and ", nBorderSurfaces, " border logical surfaces");
	}
}


G4OpticalSurface* G4World::getOrCreateOpticalSurface(const std::string& systemName, const GMirror* gmirror) {
	std::string surfaceKey = systemName + GSYSTEM_DELIMITER + gmirror->getName();

	auto cached = g4opticalSurfacesMap.find(surfaceKey);
	if (cached != g4opticalSurfacesMap.end()) { return cached->second; }

	auto* surface = new G4OpticalSurface(surfaceKey);

	// Surface type / finish / model: unknown strings are hard errors.
	auto typeIt = surfaceTypesMap().find(gmirror->getType());
	if (typeIt == surfaceTypesMap().end()) {
		log->error(ERR_G4SURFACECONFIGINVALID, "mirror <", gmirror->getName(),
		           ">: unknown surface type <", gmirror->getType(), ">");
	}
	surface->SetType(typeIt->second);

	auto finishIt = surfaceFinishesMap().find(gmirror->getFinish());
	if (finishIt == surfaceFinishesMap().end()) {
		log->error(ERR_G4SURFACECONFIGINVALID, "mirror <", gmirror->getName(),
		           ">: unknown surface finish <", gmirror->getFinish(), ">");
	}
	surface->SetFinish(finishIt->second);

	auto modelIt = surfaceModelsMap().find(gmirror->getModel());
	if (modelIt == surfaceModelsMap().end()) {
		log->error(ERR_G4SURFACECONFIGINVALID, "mirror <", gmirror->getName(),
		           ">: unknown surface model <", gmirror->getModel(), ">");
	}
	surface->SetModel(modelIt->second);

	// sigmaAlpha only applies to the unified/glisur roughness description.
	if (gmirror->hasSigmaAlpha()) { surface->SetSigmaAlpha(gmirror->getSigmaAlpha()); }

	// Boundary properties table: reuse the matOptProps material table, or build one
	// from the mirror's own property vectors (validated at GMirror construction).
	if (gmirror->usesMaterialOpticalProperties()) {
		std::string materialName = gmirror->getMatOptProps();
		G4Material* material     = nullptr;

		auto matIt = g4materialsMap.find(materialName);
		if (matIt != g4materialsMap.end()) { material = matIt->second; }
		else { material = G4NistManager::Instance()->FindMaterial(materialName); }

		if (material == nullptr) {
			log->error(ERR_G4MATERIALNOTFOUND, "mirror <", gmirror->getName(),
			           ">: matOptProps material <", materialName, "> not found");
		}
		if (material->GetMaterialPropertiesTable() == nullptr) {
			log->error(ERR_G4SURFACECONFIGINVALID, "mirror <", gmirror->getName(),
			           ">: matOptProps material <", materialName, "> has no material properties table");
		}
		surface->SetMaterialPropertiesTable(material->GetMaterialPropertiesTable());
	}
	else {
		auto* mpt = new G4MaterialPropertiesTable();

		auto photonEnergy = gmirror->getPhotonEnergy();
		auto addProperty  = [&](const char* propertyName, const std::vector<double>& values) {
			if (values.empty()) { return; }
			mpt->AddProperty(propertyName, photonEnergy, values);
		};

		addProperty("RINDEX", gmirror->getIndexOfRefraction());
		addProperty("REFLECTIVITY", gmirror->getReflectivity());
		addProperty("EFFICIENCY", gmirror->getEfficiency());
		addProperty("SPECULARLOBECONSTANT", gmirror->getSpecularLobe());
		addProperty("SPECULARSPIKECONSTANT", gmirror->getSpecularSpike());
		addProperty("BACKSCATTERCONSTANT", gmirror->getBackscatter());
		addProperty("TRANSMITTANCE", gmirror->getTransmittance());

		surface->SetMaterialPropertiesTable(mpt);
	}

	g4opticalSurfacesMap[surfaceKey] = surface;
	log->info(1, "G4World: created optical surface <", surfaceKey, "> type <", gmirror->getType(),
	          ">, finish <", gmirror->getFinish(), ">, model <", gmirror->getModel(), ">");

	return surface;
}
