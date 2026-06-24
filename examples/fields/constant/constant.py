#!/usr/bin/env python3
from pygemc import autogeometry
from pygemc import GVolume

cfg = autogeometry("examples", "constant")

world = GVolume("root")
world.description = "World for constant field display"
world.make_box(3.0, 3.0, 3.0, "m")
world.material = "G4_AIR"
world.visible = 0
world.publish(cfg)

field_volume = GVolume("constant_field_box")
field_volume.description = "Box enclosing the uniform constant field"
field_volume.make_box(1.0, 1.0, 1.0, "m")
field_volume.material = "G4_AIR"
field_volume.mfield = "constant"
field_volume.color = "orchid"
field_volume.style = 0
field_volume.opacity = 0.35
field_volume.publish(cfg)
