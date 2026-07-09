#!/usr/bin/env python3
"""Optical mirrors showcase.

Three identical 1 GeV electrons cross a gas radiator, each aimed at the center of one
reflector plate at mid-radiator. The radiator index is low, so the Cherenkov photons
stay in a narrow forward cone around each electron and land on that electron's plate.
The three plates demonstrate three different mirrors:

* polished_metal:   dielectric_metal / polished / unified skin surface with an
  explicit reflectivity table (a typical metal-coated mirror);
* rough_metal:      same skin surface but with a `ground` finish and a sigmaAlpha
  roughness, spreading the reflected image;
* semi_transparent: a half-silvered mirror defined as a border surface between the
  radiator and the plate: 60% of the photons are reflected, 30% are transmitted
  through the plate (10% are absorbed).

Each plate reflects its photons back onto its own upstream photon detector panel, so
the panels see identical conditions and only the reflection differs. A fourth panel
counts the transmitted photons after they cross a large gap behind the
semi-transparent plate. The gap is filled with an optical vacuum (refractive index 1)
so the electron radiates no Cherenkov light there and the panel sees the transmitted
photons only.
"""

from pygemc import autogeometry
from pygemc import GVolume
from pygemc import GMaterial
from pygemc import GMirror

cfg = autogeometry("examples", "mirrors")

photon_energy = "2.0*eV 3.0*eV 4.0*eV 5.0*eV 6.0*eV"

# Radiator gas: demonstration refractive index, not a real material.
radiator_gas = GMaterial("radiatorGas")
radiator_gas.description = "Demonstration radiator gas"
radiator_gas.density = 0.003
radiator_gas.addNAtoms("H", 2)
radiator_gas.photonEnergy = photon_energy
radiator_gas.indexOfRefraction = "1.0010 1.0011 1.0012 1.0013 1.0013"
radiator_gas.absorptionLength = "100*m 100*m 100*m 100*m 100*m"
radiator_gas.publish(cfg)

# Optical vacuum for the transmission gap: photons propagate (RINDEX defined) but the
# refractive index of 1 keeps the electron below the Cherenkov threshold, so the gap
# adds no photons of its own.
optical_vacuum = GMaterial("opticalVacuum")
optical_vacuum.description = "Vacuum with refractive index 1: transparent, no Cherenkov"
optical_vacuum.density = 1e-25
optical_vacuum.addMaterialWithFractionalMass("G4_Galactic", 1.0)
optical_vacuum.photonEnergy = photon_energy
optical_vacuum.indexOfRefraction = "1.0 1.0 1.0 1.0 1.0"
optical_vacuum.publish(cfg)

# Mirrors: two skin surfaces and one semi-transparent border surface.
polished = GMirror("polished_metal")
polished.description = "Polished metal mirror with explicit reflectivity"
polished.type = "dielectric_metal"
polished.finish = "polished"
polished.model = "unified"
polished.border = "SkinSurface"
polished.photonEnergy = photon_energy
polished.reflectivity = "0.90 0.90 0.89 0.88 0.87"
polished.publish(cfg)

# The specular lobe constant directs all reflection into a lobe around the facet
# normals, whose spread is sigmaAlpha; without it the unified model reflects
# diffusely (Lambertian) and the mirror image is lost entirely.
rough = GMirror("rough_metal")
rough.description = "Ground metal mirror: sigmaAlpha blurs the specular reflection"
rough.type = "dielectric_metal"
rough.finish = "ground"
rough.model = "unified"
rough.border = "SkinSurface"
rough.photonEnergy = photon_energy
rough.reflectivity = "0.90 0.90 0.89 0.88 0.87"
rough.specularlobe = "1.0 1.0 1.0 1.0 1.0"
rough.sigmaAlpha = 0.03
rough.publish(cfg)

semi = GMirror("semi_transparent")
semi.description = "Half-silvered mirror: 60% reflected, 30% transmitted, 10% absorbed"
semi.type = "dielectric_metal"
semi.finish = "polished"
semi.model = "unified"
semi.border = "semi_reflector"
semi.photonEnergy = photon_energy
semi.reflectivity = "0.60 0.60 0.60 0.60 0.60"
semi.transmittance = "0.30 0.30 0.30 0.30 0.30"
semi.publish(cfg)

# World
world = GVolume("root")
world.description = "Air world for the mirrors showcase"
world.make_box(600, 600, 600)
world.material = "G4_AIR"
world.visible = 0
world.publish(cfg)

# The radiator carries the semi_transparent border surface: it acts on photons
# crossing from the radiator into the semi_reflector plate.
radiator = GVolume("radiator")
radiator.mother = "root"
radiator.description = "1 m cube gas radiator"
radiator.make_box(500, 500, 500)
radiator.material = "radiatorGas"
radiator.color = "cyan"
radiator.style = 0
radiator.mirror = "semi_transparent"
radiator.publish(cfg)

# Three reflector plates at mid-radiator, one color per mirror. The first two are
# aluminum with skin mirrors; the semi-transparent plate is made of radiator gas so
# the transmitted photons continue undisturbed to the panel behind it.
MIRROR_COLORS = {
    "polished_metal": "gold",
    "rough_metal": "tomato",
    "semi_transparent": "deepskyblue",
}
reflectors = [
    ("flat_reflector", -320, "G4_Al", "polished_metal"),
    ("rough_reflector", 0, "G4_Al", "rough_metal"),
    ("semi_reflector", 320, "radiatorGas", None),
]
for name, x, material, mirror_name in reflectors:
    plate = GVolume(name)
    plate.mother = "radiator"
    plate.description = f"Reflector plate at x = {x} mm"
    plate.make_box(140, 440, 5)
    plate.set_position(x, 0, 300)
    plate.material = material
    plate.color = MIRROR_COLORS[mirror_name or "semi_transparent"]
    if mirror_name is not None:
        plate.mirror = mirror_name
    plate.publish(cfg)

# Transmission gap behind the semi-transparent plate: a long stretch of optical
# vacuum that lets the transmitted photons be seen in the display without adding
# Cherenkov light from the electron.
gap = GVolume("transmission_gap")
gap.mother = "radiator"
gap.description = "Optical vacuum gap behind the semi-transparent plate"
gap.make_box(140, 440, 90)
gap.set_position(320, 0, 396)
gap.material = "opticalVacuum"
gap.color = "whitesmoke"
gap.style = 0
gap.publish(cfg)

# One upstream photon detector per reflector, aligned with it and sharing its mirror
# color, so each panel counts the photons reflected by its own mirror under identical
# conditions. A fourth panel counts the photons transmitted across the gap behind the
# semi-transparent plate.
# Same material as the radiator so optical photons are not killed at its boundary.
detectors = [
    ("photon_detector_flat", -320, -490, 1, "gold"),
    ("photon_detector_rough", 0, -490, 2, "tomato"),
    ("photon_detector_semi", 320, -490, 3, "deepskyblue"),
    ("photon_detector_behind", 320, 492, 4, "deepskyblue"),
]
for name, x, z, panel_id, color in detectors:
    detector = GVolume(name)
    detector.mother = "radiator"
    detector.description = f"Optical photon collection panel at x = {x} mm, z = {z} mm"
    detector.make_box(140, 440, 1)
    detector.set_position(x, 0, z)
    detector.material = "radiatorGas"
    detector.color = color
    detector.digitization = "gPhotonDetector"
    detector.set_identifier("panel", panel_id)
    detector.publish(cfg)
