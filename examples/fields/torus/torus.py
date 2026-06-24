#!/usr/bin/env python3
from pygemc import autogeometry
from pygemc import GVolume

cfg = autogeometry("examples", "torus")

world = GVolume("root")
world.description = "World for torus field display"
world.make_box(7.0, 7.0, 7.0, "m")
world.material = "G4_AIR"
world.visible = 0
world.publish(cfg)

field_volume = GVolume("torus_field_volume")
field_volume.description = "Toroidal volume enclosing the torus ASCII field map"
field_volume.solid = "G4Torus"
field_volume.parameters = "0*cm, 260*cm, 275*cm, 0*deg, 360*deg"
field_volume.material = "G4_AIR"
field_volume.mfield = "torus"
field_volume.set_position(0, 0, 350, "cm")
field_volume.color = "gold"
field_volume.style = 0
field_volume.opacity = 0.35
field_volume.publish(cfg)
