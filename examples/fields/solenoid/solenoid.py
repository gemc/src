#!/usr/bin/env python3
from pygemc import autogeometry
from pygemc import GVolume

cfg = autogeometry("examples", "solenoid")

world = GVolume("root")
world.description = "World for solenoid field display"
world.make_box(7.2, 7.2, 7.2, "m")
world.material = "G4_AIR"
world.visible = 0
world.publish(cfg)

field_volume = GVolume("solenoid_field_tube")
field_volume.description = "Cylindrical volume enclosing the solenoid ASCII field map"
field_volume.make_tube(0, 3.0, 3.0, 0, 360, "m", "deg")
field_volume.material = "G4_AIR"
field_volume.mfield = "solenoid"
field_volume.color = "seagreen"
field_volume.style = 0
field_volume.opacity = 0.3
field_volume.publish(cfg)
