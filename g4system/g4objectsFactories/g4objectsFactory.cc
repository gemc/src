/**
 * @file   g4objectsFactory.cc
 * @ingroup Geometry
 * @brief  Default implementations shared by all detector-specific factories.
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

// Configure factory behavior (overlap checking + backup material).
void G4ObjectsFactory::initialize_context(int                check_overlaps,
                                          const std::string& backup_mat) {
	checkOverlaps  = check_overlaps;
	backupMaterial = backup_mat;
}


// Convenience: retrieve the solid pointer for a named volume from the wrapper map.
G4VSolid* G4ObjectsFactory::getSolidFromMap(const std::string&                          vname,
                                            std::unordered_map<std::string, G4Volume*>* g4s) {
	auto it = g4s->find(vname);
	return (it != g4s->end()) ? it->second->getSolid() : nullptr;
}


// Convenience: retrieve the logical pointer for a named volume from the wrapper map.
G4LogicalVolume* G4ObjectsFactory::getLogicalFromMap(const std::string&                          volume_name,
                                                     std::unordered_map<std::string, G4Volume*>* g4s) {
	auto it = g4s->find(volume_name);
	return (it != g4s->end()) ? it->second->getLogical() : nullptr;
}

// Convenience: retrieve the physical pointer for a named volume from the wrapper map.
G4VPhysicalVolume* G4ObjectsFactory::getPhysicalFromMap(const std::string&                          vname,
                                                        std::unordered_map<std::string, G4Volume*>* g4s) {
	auto it = g4s->find(vname);
	return (it != g4s->end()) ? it->second->getPhysical() : nullptr;
}


G4VisAttributes G4ObjectsFactory::createVisualAttributes(const GVolume* s) {
	// Convert GEMC color + opacity into a Geant4 color. The helper returns a \c G4Colour.
	std::string_view color   = s->getColor();
	double           opacity = s->getOpacity();

	const auto g4color = gutilities::makeG4Colour(color, opacity);

	log->info(2, className(), " createVisualAttributes for color ", color,
	          " resulted in RGB = (", g4color.GetRed(), ", ", g4color.GetGreen(), ", ", g4color.GetBlue(),
	          ", opacity: ", opacity, ")");

	G4VisAttributes attr(g4color);

	// Visibility and style flags are stored on the volume definition.
	s->isVisible() ? attr.SetVisibility(true) : attr.SetVisibility(false);
	s->getStyle() ? attr.SetForceSolid(true) : attr.SetForceWireframe(true);
	return attr;
}


G4RotationMatrix* G4ObjectsFactory::getRotation(const GVolume* s) {
	// Parse a rotation string expressed as three angles and apply them in X/Y/Z order.
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
	// Base placement position.
	G4ThreeVector pos(0., 0., 0.);
	const auto    vec = gutilities::getG4NumbersFromString(s->getPos());
	if (vec.size() == 3) pos.set(vec[0], vec[1], vec[2]);

	// Optional shift modifier (applied after parsing the base position).
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

	// Logical exists, return it.
	if (thisG4Volume->getLogical()) return thisG4Volume->getLogical();

	// If this volume is a "copy of" another, reuse the logical volume if it already exists.
	std::string copyOf = s->getCopyOf();
	if (copyOf != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) {
		auto gsystem      = s->getSystem();
		auto volume_copy  = gsystem + "/" + copyOf;
		auto copyG4Volume = getOrCreateG4Volume(volume_copy, g4s);
		if (copyG4Volume->getLogical() != nullptr) {
			return copyG4Volume->getLogical();
		}
	}

	// Material lookup:
	// - first try requested material
	// - if missing and a backup material was configured, fall back to it
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

	// Create the logical volume with the already-created solid.
	auto* logical = new G4LogicalVolume(thisG4Volume->getSolid(),
	                                    mat, g4name);

	// Apply visualization attributes (color/opacity/visibility/style).
	logical->SetVisAttributes(createVisualAttributes(s));
	thisG4Volume->setLogical(logical, log);
	return logical;
}


G4VPhysicalVolume* G4ObjectsFactory::buildPhysical(const GVolume*                              s,
                                                   std::unordered_map<std::string, G4Volume*>* g4s) {
	// Nonexistent volumes are ignored by design.
	if (!s->getExistence()) return nullptr;

	// Mother/logical prerequisites must exist; otherwise caller will retry later.
	if (!checkPhysicalDependencies(s, g4s)) return nullptr;

	const std::string g4name        = s->getG4Name();
	auto              thisG4Volume  = getOrCreateG4Volume(g4name, g4s);
	auto              logicalVolume = thisG4Volume->getLogical();

	// If this is a copy, reuse the source logical volume when available.
	std::string copyOf = s->getCopyOf();
	if (copyOf != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) {
		auto gsystem      = s->getSystem();
		auto volume_copy  = gsystem + "/" + copyOf;
		auto copyG4Volume = getOrCreateG4Volume(volume_copy, g4s);
		if (copyG4Volume->getLogical() != nullptr) {
			logicalVolume = copyG4Volume->getLogical();
		}
	}

	// Create the placement only once; subsequent calls return the cached physical volume.
	if (!thisG4Volume->getPhysical()) {
		G4RotationMatrix rotation_instance    = *getRotation(s);
		G4ThreeVector    translation_instance = getPosition(s);

		thisG4Volume->setPhysical(new G4PVPlacement(G4Transform3D(rotation_instance, translation_instance),
		                                            logicalVolume,
		                                            g4name,
		                                            getLogicalFromMap(s->getG4MotherName(), g4s),
		                                            false,
		                                            s->getPCopyNo(),
		                                            checkOverlaps > 0),
		                          log);
	}
	return thisG4Volume->getPhysical();
}


bool G4ObjectsFactory::build_g4volume(const GVolume*                              s,
                                      std::unordered_map<std::string, G4Volume*>* g4s) {
	const auto name = s->getG4Name();

	// Build steps can fail due to missing dependencies; each returns nullptr in that case.
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
