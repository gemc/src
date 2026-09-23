#!/usr/bin/env python3
"""A tube target that records the generated geantino's original vertex (geometry lengths in mm)."""

from pygemc import GVolume, autogeometry

cfg = autogeometry("examples", "generator_manipulations")

world = GVolume("root")
world.description = "Generator manipulation world"
world.make_box(60, 60, 60)
world.material = "G4_Galactic"
world.style = 0
world.publish(cfg)

target = GVolume("target")
target.mother = "root"
target.description = "Liquid hydrogen tube target"
target.make_tube(0, 25, 20, 0, 360)
target.material = "G4_lH2"
target.color = "darkgreen"
target.opacity = 0.2
target.digitization = "flux"
target.set_identifier("target", 1)
target.publish(cfg)
