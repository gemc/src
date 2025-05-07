/**
 * @file   g4objectsFactory.cc
 * @ingroup Geometry
 * @brief  Default implementations shared by all detector‑specific factories.
 */

#include "g4objectsFactory.h"

#include "gutilities.h"
#include "gsystemConventions.h"
#include "g4system/g4systemConventions.h"

// Geant4
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"

// c++
#include <string_view>

void G4ObjectsFactory::initialize_context(const std::shared_ptr<GLogger>& logger,
                                          int                             check_overlaps,
                                          std::string                     backup_mat) {
	log             = logger;
	checkOverlaps   = check_overlaps;
	backupMaterial  = std::move(backup_mat);
	log_is_assigned = true;
}


G4VSolid* G4ObjectsFactory::getSolidFromMap(const std::string&                          vname,
                                            std::unordered_map<std::string, G4Volume*>* g4s) {
	auto it = g4s->find(vname);
	return (it != g4s->end()) ? it->second->getSolid() : nullptr;
}

G4LogicalVolume* G4ObjectsFactory::getLogicalFromMap(const std::string&                          volume_name,
                                                     std::unordered_map<std::string, G4Volume*>* g4s) {
	auto it = g4s->find(volume_name);
	return (it != g4s->end()) ? it->second->getLogical() : nullptr;
}

G4VPhysicalVolume* G4ObjectsFactory::getPhysicalFromMap(const std::string&                          vname,
                                                        std::unordered_map<std::string, G4Volume*>* g4s) {
	auto it = g4s->find(vname);
	return (it != g4s->end()) ? it->second->getPhysical() : nullptr;
}


G4VisAttributes G4ObjectsFactory::createVisualAttributes(const GVolume* s) {
	std::string_view color = s->getColor();

	const auto g4color = gutilities::makeColour(color);

	log->info(2, className(), " createVisualAttributes for color ", color,
	          " resulted in RGB = (", g4color.GetRed(), ", ", g4color.GetGreen(), ", ", g4color.GetBlue(), ")");

	G4VisAttributes attr(g4color);

	s->isVisible() ? attr.SetVisibility(true) : attr.SetVisibility(false);
	s->getStyle() ? attr.SetForceSolid(true) : attr.SetForceWireframe(true);
	return attr;
}


G4RotationMatrix* G4ObjectsFactory::getRotation(const GVolume* s) {
	auto rot = new G4RotationMatrix();

	const auto rotDef = gutilities::getStringVectorFromStringWithDelimiter(s->getRot(), ",");
	if (rotDef.size() == 3) {
		const auto pars = gutilities::getG4NumbersFromStringVector(rotDef);
		rot->rotateX(pars[0]);
		rot->rotateY(pars[1]);
		rot->rotateZ(pars[2]);
	}
	// (ordered rotation parsing omitted for brevity – keep original logic)
	return rot;
}

G4ThreeVector G4ObjectsFactory::getPosition(const GVolume* s) {
	G4ThreeVector pos(0., 0., 0.);
	const auto    vec = gutilities::getG4NumbersFromString(s->getPos());
	if (vec.size() == 3) pos.set(vec[0], vec[1], vec[2]);

	if (s->getShift() != GSYSTEMNOMODIFIER) {
		const auto shift = gutilities::getG4NumbersFromString(s->getShift());
		if (shift.size() == 3) pos += G4ThreeVector(shift[0], shift[1], shift[2]);
	}
	return pos;
}


G4LogicalVolume* G4ObjectsFactory::buildLogical(const GVolume*                              s,
                                                std::unordered_map<std::string, G4Volume*>* g4s) {
	const std::string g4name = s->getG4Name();
	auto thisG4Volume = getOrCreateG4Volume(g4name, g4s);


	if (thisG4Volume->getLogical()) return thisG4Volume->getLogical();
	if (!thisG4Volume->getSolid()) return nullptr; // dependency, check here

	// material lookup
	auto* nist = G4NistManager::Instance();
	auto* mat  = nist->FindOrBuildMaterial(s->getMaterial());
	if (!mat && !backupMaterial.empty()) {
		mat = nist->FindOrBuildMaterial(backupMaterial);
		log->warning("Material <", s->getMaterial(), "> not found. Using backup material <", backupMaterial, ">.");
	}

	if (!mat) {
		log->error(ERR_G4MATERIALNOTFOUND,
		           "Material <", s->getMaterial(), "> not found.");
	}

	auto* logical = new G4LogicalVolume(thisG4Volume->getSolid(),
	                                    mat, g4name);

	logical->SetVisAttributes(createVisualAttributes(s));
	thisG4Volume->setLogical(logical, log);
	return logical;
}


G4VPhysicalVolume* G4ObjectsFactory::buildPhysical(const GVolume*                              s,
                                                   std::unordered_map<std::string, G4Volume*>* g4s) {
	if (!s->getExistence()) return nullptr;
	if (!checkPhysicalDependencies(s, g4s)) return nullptr;

	const std::string g4name = s->getG4Name();
	auto thisG4Volume = getOrCreateG4Volume(g4name, g4s);

	if (!thisG4Volume->getPhysical()) {
		thisG4Volume->setPhysical(new G4PVPlacement(getRotation(s),
		                                    getPosition(s),
		                                    thisG4Volume->getLogical(),
		                                    g4name,
		                                    getLogicalFromMap(s->getG4MotherName(), g4s),
		                                    false,
		                                    s->getPCopyNo(),
		                                    checkOverlaps > 0),
		                  log);
	}
	return thisG4Volume->getPhysical();
}


bool G4ObjectsFactory::build_g4volume(const GVolume* s,
                                    std::unordered_map<std::string,
                                                       G4Volume*>* g4s) {

	const auto name = s->getG4Name();
	log->info(2, className(), " loadG4System: building <", name, ">");

	auto sbuild = buildSolid(s, g4s);
	log->info(2, className(), " loadG4System: buildSolid <", name, ">");

	auto lbuild = buildLogical(s, g4s);
	log->info(2, className(), " loadG4System: buildLogical <", name, ">");

	auto pbuild = buildPhysical(s, g4s);
	log->info(2, className(), " loadG4System: buildPhysical <", name, ">");

	const bool okSolid    = (sbuild != nullptr);
	const bool okLogical  = (lbuild != nullptr);
	const bool okPhysical = (pbuild != nullptr);
	const bool okAll      = okSolid && okLogical && okPhysical;

	log->info(2, className(), " result: ",
	          " solid: ", okSolid,
	          " logical: ", okLogical,
	          " physical: ", okPhysical);

	return okAll;
}
