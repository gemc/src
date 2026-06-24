#!/usr/bin/env python3
from pygemc import autogeometry
from pygemc import GVolume

cfg = autogeometry("examples", "dipole")

world = GVolume("root")
world.description = "World for dipole field display"
world.make_box(1.8, 1.2, 1.2, "m")
world.material = "G4_AIR"
world.visible = 0
world.publish(cfg)

field_volume = GVolume("dipole_field_box")
field_volume.description = "Box enclosing the ideal dipole field region"
field_volume.make_box(75, 35, 35, "cm")
field_volume.material = "G4_AIR"
field_volume.mfield = "dipole"
field_volume.color = "cornflowerblue"
field_volume.style = 0
field_volume.opacity = 0.35
field_volume.publish(cfg)
