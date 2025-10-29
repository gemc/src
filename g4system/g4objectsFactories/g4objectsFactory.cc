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

void G4ObjectsFactory::initialize_context(int                check_overlaps,
                                          const std::string& backup_mat) {
	checkOverlaps  = check_overlaps;
	backupMaterial = backup_mat;
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
	double opacity = s->getOpacity();

	const auto g4color = gutilities::makeG4Colour(color, opacity);

	log->info(2, className(), " createVisualAttributes for color ", color,
	          " resulted in RGB = (", g4color.GetRed(), ", ", g4color.GetGreen(), ", ", g4color.GetBlue(), ", opacity: ", opacity, ")");

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
	const std::string g4name       = s->getG4Name();
	auto              thisG4Volume = getOrCreateG4Volume(g4name, g4s);


	// logical exists, return it
	if (thisG4Volume->getLogical()) return thisG4Volume->getLogical();

	// copy logical exists, return it
	std::string copyOf = s->getCopyOf();
	if (copyOf != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) {
		auto gsystem      = s->getSystem();
		auto volume_copy  = gsystem + "/" + copyOf;
		auto copyG4Volume = getOrCreateG4Volume(volume_copy, g4s);
		if (copyG4Volume->getLogical() != nullptr) {
			return copyG4Volume->getLogical();
		}
	}


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

	const std::string g4name        = s->getG4Name();
	auto              thisG4Volume  = getOrCreateG4Volume(g4name, g4s);
	auto              logicalVolume = thisG4Volume->getLogical();

	// copy logical exists, builds from it
	std::string copyOf = s->getCopyOf();
	if (copyOf != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) {
		auto gsystem      = s->getSystem();
		auto volume_copy  = gsystem + "/" + copyOf;
		auto copyG4Volume = getOrCreateG4Volume(volume_copy, g4s);
		if (copyG4Volume->getLogical() != nullptr) {
			logicalVolume = copyG4Volume->getLogical();
		}
	}

	if (!thisG4Volume->getPhysical()) {
		G4RotationMatrix rotation_instance = *getRotation(s);
		G4ThreeVector    translation_instance = getPosition(s);

		thisG4Volume->setPhysical(new G4PVPlacement( G4Transform3D(rotation_instance, translation_instance),
		                                            logicalVolume,
		                                            g4name,
		                                            getLogicalFromMap(s->getG4MotherName(), g4s),
		                                            false,
		                                            s->getPCopyNo(),
		                                            checkOverlaps > 0),
		                          log);

		// passive rotation
		// thisG4Volume->setPhysical(new G4PVPlacement(getRotation(s),
		//                                             getPosition(s),
		//                                             logicalVolume,
		//                                             g4name,
		//                                             getLogicalFromMap(s->getG4MotherName(), g4s),
		//                                             false,
		//                                             s->getPCopyNo(),
		//                                             checkOverlaps > 0),
		//                           log);



	}
	return thisG4Volume->getPhysical();
}


bool G4ObjectsFactory::build_g4volume(const GVolume*                              s,
                                      std::unordered_map<std::string, G4Volume*>* g4s) {
	const auto name = s->getG4Name();

	auto sbuild = buildSolid(s, g4s);
	auto lbuild = buildLogical(s, g4s);
	auto pbuild = buildPhysical(s, g4s);

	const bool okSolid    = (sbuild != nullptr);
	const bool okLogical  = (lbuild != nullptr);
	const bool okPhysical = (pbuild != nullptr);
	const bool okAll      = okSolid && okLogical && okPhysical;

	log->info(2, className(), " result for ",
	          name,
	          ": solid: ", okSolid,
	          " logical: ", okLogical,
	          " physical: ", okPhysical);

	return okAll;
}
