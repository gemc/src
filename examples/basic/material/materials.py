#!/usr/bin/env python3
from pygemc import autogeometry
from pygemc import GVolume
from pygemc import GMaterial

cfg = autogeometry("examples", "material")

world_size = 50
world = GVolume("root")
world.description = "World"
world.make_box(world_size * 0.5, world_size * 0.5, world_size * 0.5)
world.material = "G4_AIR"
world.color = "ghostwhite"
world.visible = 0
world.publish(cfg)

# Five tubes: 10 cm diameter, 1 cm thick, along z-axis starting at z=0.
# Each tube is separated from the next by 3 cm (edge to edge), so centers
# are 4 cm apart (1 cm thickness + 3 cm gap).
radius         = 5    # cm, giving 10 cm diameter
half_thickness = 0.5  # cm, giving 1 cm total thickness
z_step         = 4    # cm center-to-center spacing

# --- Tube 1: Geant4 built-in material ---
tube1 = GVolume("tube_carbon")
tube1.mother = "root"
tube1.description = "Carbon tube using the Geant4 built-in G4_C (graphite) material"
tube1.make_tube(0, radius, half_thickness, 0, 360)
tube1.material = "G4_C"
tube1.color = "808080"
tube1.set_position(0, 0, 0.5)
tube1.publish(cfg)

# --- Tube 2: Custom material by molecular composition ---
water = GMaterial("custom_water")
water.description = "Water defined by molecular composition (H2O)"
water.density = 1.0
water.addNAtoms("H", 2)
water.addNAtoms("O", 1)
water.publish(cfg)

tube2 = GVolume("tube_water")
tube2.mother = "root"
tube2.description = "Tube made of custom water defined by molecular composition"
tube2.make_tube(0, radius, half_thickness, 0, 360)
tube2.material = "custom_water"
tube2.color = "1565C0"
tube2.set_position(0, 0, 0.5 + z_step)
tube2.publish(cfg)

# --- Tube 3: Custom material by fractional masses ---
mixture = GMaterial("air_water_mixture")
mixture.description = "80% air / 20% water mixture defined by fractional masses"
mixture.density = 0.9601
mixture.addMaterialWithFractionalMass("G4_AIR",   0.80)
mixture.addMaterialWithFractionalMass("G4_WATER", 0.20)
mixture.publish(cfg)

tube3 = GVolume("tube_mixture")
tube3.mother = "root"
tube3.description = "Tube made of an air/water mixture defined by fractional masses"
tube3.make_tube(0, radius, half_thickness, 0, 360)
tube3.material = "air_water_mixture"
tube3.color = "2E7D32"
tube3.set_position(0, 0, 0.5 + 2 * z_step)
tube3.publish(cfg)

# --- Tube 4: Scintillator with fast/slow emission components ---
scint_energy = "2.0*eV 2.5*eV 3.0*eV 3.5*eV 4.0*eV"

scintillator = GMaterial("my_scintillator")
scintillator.description = "NaI-like scintillator with fast and slow emission components"
scintillator.density = 3.67
scintillator.addNAtoms("Na", 1)
scintillator.addNAtoms("I",  1)
scintillator.photonEnergy       = scint_energy
scintillator.fastcomponent      = "1.0 0.9 0.8 0.7 0.6"
scintillator.slowcomponent      = "0.5 0.4 0.3 0.2 0.1"
scintillator.scintillationyield = 1000
scintillator.resolutionscale    = 1.0
scintillator.fasttimeconstant   = 6
scintillator.slowtimeconstant   = 88
scintillator.yieldratio         = 0.8
scintillator.birksConstant      = 0.00152
scintillator.publish(cfg)

tube4 = GVolume("tube_scintillator")
tube4.mother = "root"
tube4.description = "Tube made of a NaI-like scintillating material"
tube4.make_tube(0, radius, half_thickness, 0, 360)
tube4.material = "my_scintillator"
tube4.color = "F57F17"
tube4.style = 0
tube4.set_position(0, 0, 0.5 + 3 * z_step)
tube4.publish(cfg)

# --- Tube 5: Optical glass with index of refraction (Cherenkov radiator) ---
photon_energy = "2.0*eV 3.0*eV 4.0*eV 5.0*eV"

optical_glass = GMaterial("optical_glass")
optical_glass.description = "SiO2 optical glass with a refractive index for Cherenkov applications"
optical_glass.density = 2.5
optical_glass.addNAtoms("Si", 1)
optical_glass.addNAtoms("O",  2)
optical_glass.photonEnergy      = photon_energy
optical_glass.indexOfRefraction = "1.458 1.466 1.476 1.490"
optical_glass.absorptionLength  = "3*m 3*m 3*m 3*m"
optical_glass.publish(cfg)

tube5 = GVolume("tube_optical")
tube5.mother = "root"
tube5.description = "Tube made of SiO2 optical glass with a refractive index for Cherenkov radiation"
tube5.make_tube(0, radius, half_thickness, 0, 360)
tube5.material = "optical_glass"
tube5.color = "00BCD4"
tube5.style = 2
tube5.set_position(0, 0, 0.5 + 4 * z_step)
tube5.publish(cfg)
