#!/usr/bin/env python3
"""A tube target that records the incident geantino's original vertex (geometry lengths in mm)."""

from pygemc import GVolume, autogeometry

cfg = autogeometry("examples", "vertex_manipulation")

world = GVolume("root")
world.description = "Vertex manipulation world"
world.make_box(600, 600, 600)
world.material = "G4_Galactic"
world.style = 0
world.publish(cfg)

target = GVolume("target")
target.mother = "root"
target.description = "Liquid hydrogen tube target"
target.make_tube(0, 25, 20, 0, 360)
target.material = "G4_lH2"
target.color = "darkgreen"
target.digitization = "flux"
target.set_identifier("target", 1)
target.publish(cfg)
