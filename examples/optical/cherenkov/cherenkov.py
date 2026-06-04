#!/usr/bin/env python3

from pygemc import autogeometry
from pygemc import GVolume
from pygemc import GMaterial

cfg = autogeometry("examples", "cherenkov")

# Using 3 demonstration radiators with neutral names. Their optical constants
# are for demonstration and are not intended to define real materials.
variation_material = {
    "default": "lowIndexRadiator",
    "mediumIndexRadiator": "mediumIndexRadiator",
    "highIndexRadiator": "highIndexRadiator",
}

photon_energy = "2.0*eV 3.0*eV 4.0*eV 5.0*eV 6.0*eV"

# Optical properties
# Refractive index and absorption at 2, 3, 4, 5, 6 eV.
# These refractive-index tables are demonstration values and avoid material
# names that imply a specific real-world gas.
rindex = {
    "lowIndexRadiator":    "1.0010 1.0011 1.0012 1.0013 1.0013",
    "mediumIndexRadiator": "1.0110 1.0120 1.0130 1.0140 1.0150",
    "highIndexRadiator":   "1.0500 1.0510 1.0520 1.0530 1.0530",
}
absorption = {
    "lowIndexRadiator": "100*m       100*m       100*m       100*m       100*m",
    "mediumIndexRadiator": "50*m        50*m        40*m        30*m        20*m",
    "highIndexRadiator": "6*m        6*m         6*m         5*m         4*m",
}
colors = {
    "lowIndexRadiator": "cyan",
    "mediumIndexRadiator": "blue",
    "highIndexRadiator": "green"
}

material_definitions = {
    "lowIndexRadiator": {
        "name": "lowIndexRadiator",
        "description": "Low-index demonstration radiator",
        "density": 0.003,
        "atoms": [("H", 2)],
    },

    "mediumIndexRadiator": {
        "name": "mediumIndexRadiator",
        "description": "Medium-index demonstration radiator",
        "density": 0.003,
        "atoms": [("H", 2)],
    },

    "highIndexRadiator": {
        "name": "highIndexRadiator",
        "description": "High-index demonstration radiator",
        "density": 0.003,
        "atoms": [("H", 2)],
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

    for material_name, definition in material_definitions.items():
        mat = GMaterial(definition["name"])
        mat.description = definition["description"]
        mat.density = definition["density"]

        for element, natoms in definition["atoms"]:
            mat.addNAtoms(element, natoms)

        mat.photonEnergy = photon_energy
        mat.indexOfRefraction = rindex[material_name]
        mat.absorptionLength = absorption[material_name]
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
        backplate.make_box(half_x, half_y, 1)
        backplate.set_position(x, y, 499)

        # Use the same material as the radiator so optical photons are not killed
        # by crossing into a material without matching optical properties.
        backplate.material = material

        backplate.color = "lightgray"
        backplate.digitization = "gPhotonDetector"
        backplate.set_identifier("detector", panel_id)
        backplate.publish(cfg)
