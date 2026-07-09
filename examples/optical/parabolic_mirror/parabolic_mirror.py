#!/usr/bin/env python3
"""Parabolic optical mirror example.

A parallel bundle of optical photons enters an optical vacuum volume, reflects from
the inner surface of a thin Geant4 G4Paraboloid mirror shell, and is collected by one
small photon detector near the nominal focus. The layout emulates a simple telescope
dish.
"""

from pygemc import autogeometry
from pygemc import GVolume
from pygemc import GMaterial
from pygemc import GMirror

cfg = autogeometry("examples", "parabolic_mirror")

photon_energy = "2.0*eV 3.0*eV 4.0*eV 5.0*eV 6.0*eV"

optical_vacuum = GMaterial("opticalVacuum")
optical_vacuum.description = "Vacuum with refractive index 1 for optical photon transport"
optical_vacuum.density = 1e-25
optical_vacuum.addMaterialWithFractionalMass("G4_Galactic", 1.0)
optical_vacuum.photonEnergy = photon_energy
optical_vacuum.indexOfRefraction = "1.0 1.0 1.0 1.0 1.0"
optical_vacuum.absorptionLength = "100*m 100*m 100*m 100*m 100*m"
optical_vacuum.publish(cfg)

reflector = GMirror("parabolic_reflector")
reflector.description = "Polished metal coating on the parabolic mirror"
reflector.type = "dielectric_metal"
reflector.finish = "polished"
reflector.model = "unified"
reflector.border = "SkinSurface"
reflector.photonEnergy = photon_energy
reflector.reflectivity = "0.95 0.95 0.95 0.95 0.95"
reflector.publish(cfg)

world = GVolume("root")
world.description = "Air world for the parabolic mirror example"
world.make_box(800, 800, 800)
world.material = "G4_AIR"
world.visible = 0
world.publish(cfg)

space = GVolume("optical_space")
space.mother = "root"
space.description = "Optical vacuum around the mirror and detector"
space.make_box(650, 650, 650)
space.material = "opticalVacuum"
space.visible = 0
space.publish(cfg)

mirror_dz = 30.0
vertex_radius = 20.0
rim_radius = 220.0
mirror_z = 180.0
shell_thickness = 2.0

# G4Paraboloid requires the radius at -dz to be smaller than the radius at +dz.
# The shell is rotated 180 degrees around x so the inner mirror surface opens toward
# incoming photons at negative global z. For the dominant radius, f ~= R^2 / (4 * depth).
focus_z = mirror_z + mirror_dz - rim_radius * rim_radius / (4.0 * (2.0 * mirror_dz))

outer_dish = GVolume("outer_dish")
outer_dish.mother = "optical_space"
outer_dish.description = "Outer G4Paraboloid operand for the parabolic mirror shell"
outer_dish.make_paraboloid(mirror_dz, vertex_radius, rim_radius)
outer_dish.material = "G4_Al"
outer_dish.visible = 0
outer_dish.exist = 0
outer_dish.publish(cfg)

inner_dish = GVolume("inner_dish")
inner_dish.mother = "optical_space"
inner_dish.description = "Inner G4Paraboloid operand subtracted to make the mirror cavity"
inner_dish.make_paraboloid(
    mirror_dz ,
    vertex_radius - shell_thickness,
    rim_radius - shell_thickness,
)
inner_dish.material = "opticalVacuum"
inner_dish.visible = 0
inner_dish.exist = 0
inner_dish.publish(cfg)

mirror = GVolume("parabolic_dish")
mirror.mother = "optical_space"
mirror.description = "Thin boolean shell whose inner cavity surface is a polished mirror"
mirror.solidsOpr = "outer_dish - inner_dish"
mirror.set_position(0, 0, mirror_z)
mirror.set_rotation(180, 0, 0)
mirror.material = "G4_Al"
mirror.color = "silver"
mirror.mirror = "parabolic_reflector"
mirror.publish(cfg)

detector = GVolume("focus_detector")
detector.mother = "optical_space"
detector.description = "Small optical photon detector at the nominal parabolic focus"
detector.make_tube(0, 10, 1, 0, 360)
detector.set_position(0, 0, focus_z)
# same optical vacuum as the surroundings: a material without a refractive index
# (e.g. G4_AIR) would kill the photons at the detector boundary before they count
detector.material = "opticalVacuum"
detector.color = "deepskyblue"
detector.digitization = "gPhotonDetector"
detector.set_identifier("panel", 1)
detector.publish(cfg)
