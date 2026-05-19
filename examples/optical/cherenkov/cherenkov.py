#!/usr/bin/env python3

from gconfiguration import autogeometry
from gvolume import GVolume
from gmaterial import GMaterial

cfg = autogeometry("examples", "cherenkov")

# Using 3 gases with different refractive indices
variation_material = {
	"default": "CF4",
	"CO2": "CO2",
	"C4F10": "C4F10",
}

photon_energy = "2.0*eV 3.0*eV 4.0*eV 5.0*eV 6.0*eV"

# Optical properties
# Refractive index and absorbtion at 2, 3, 4, 5, 6 eV
rindex = {
	"CF4": "1.000481811 1.000487934 1.000496771 1.000508615 1.000523881",
	"CO2": "1.000448007 1.000458006 1.000472449 1.000492454 1.000519671",
	"C4F10": "1.001306001 1.001329786 1.001364578 1.001412079 1.001474826",
}
absorption = {
	"CF4": "100*m       100*m       100*m       100*m       100*m",
	"CO2": "50*m        50*m        40*m        30*m        20*m",
	"C4F10": "6*m        6*m         6*m         5*m         4*m",
}
colors = {
	"CF4": "red",
	"CO2": "blue",
	"C4F10": "green"
}

material_definitions = {
	"CF4": {
		"name": "CF4",
		"description": "CF4 gas with optical properties",
		# g/cm3, approximately 20 C and 1 atm
		"density": 0.003658,
		"atoms": [("C", 1), ("F", 4)],
	},

	"CO2": {
		"name": "CO2",
		"description": "CO2 gas at 1 atm with optical properties",
		"density": 0.001842,
		"atoms": [("C", 1), ("O", 2)],
	},

	"C4F10": {
		"name": "C4F10",
		"description": "C4F10 gas with optical properties",
		"density": 0.00973,
		"atoms": [("C", 4), ("F", 10)],
	},
}

# common panel geometry
panels = [
	("detector_left", -255, 0, 245, 500, 1),
	("detector_right", 255, 0, 245, 500, 2),
	("detector_bottom", 0, -255, 500, 245, 3),
	("detector_top", 0, 255, 500, 245, 4),
]

for variation, material in variation_material.items():
	cfg.init_variation(variation)

	mat = GMaterial(material_definitions[material]["name"])
	mat.description = material_definitions[material]["description"]
	mat.density = material_definitions[material]["density"]

	for element, natoms in material_definitions[material]["atoms"]:
		mat.addNAtoms(element, natoms)

	mat.photonEnergy = photon_energy
	mat.indexOfRefraction = rindex[material]
	mat.absorptionLength = absorption[material]
	mat.publish(cfg)

	# World
	world = GVolume("root")
	world.description = "Air world for Cherenkov validation"
	world.make_box(600, 600, 600)
	world.material = "G4_AIR"
	world.visible = 0
	world.publish(cfg)

	radiator = GVolume("radiator")
	radiator.mother = "root"
	radiator.description = f"1 m cube radiator using {material}"
	radiator.make_box(500, 500, 500)
	radiator.material = material
	radiator.color = colors[material]
	radiator.style = 2
	radiator.publish(cfg)

	for name, x, y, half_x, half_y, panel_id in panels:
		backplate = GVolume(name)
		backplate.variation = variation
		backplate.mother = "radiator"
		backplate.description = "Downstream optical photon collection panel"
		backplate.make_box(half_x, half_y, 0.5)
		backplate.set_position(x, y, 499)

		# Use same gas as the radiator so optical photons are not killed
		# by crossing into a material without matching optical properties.
		backplate.material = material

		backplate.color = "lightpink"
		backplate.digitization = "flux"
		backplate.set_identifier("detector", panel_id)
		backplate.publish(cfg)
