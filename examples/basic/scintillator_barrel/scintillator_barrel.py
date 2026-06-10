#!/usr/bin/env python3
import math
from pygemc import autogeometry, GVolume

cfg = autogeometry("examples", "scintillator_barrel")

n      = 48
radius = 400.0   # mm — radial position of each paddle center
pZ     = 500.0   # mm — half-length along the beam axis (100 cm total)
pY     =  20.0   # mm — radial half-thickness

# Tangential half-widths sized so adjacent paddles meet at the outer face.
# The chord half-length at radius r for an angular step 2π/n is r·sin(π/n).
half_step = math.radians(180.0 / n)
pX    = (radius + pY) * math.sin(half_step)   # outer tangential half-width (wider face)
pLTX  = (radius - pY) * math.sin(half_step)   # inner tangential half-width (narrow face)

world_xy = radius + pY + 30
world_z  = pZ + 30

world = GVolume("root")
world.description = "World volume"
world.make_box(world_xy, world_xy, world_z)
world.material = "G4_AIR"
world.color = "ghostwhite"
world.style = 0
world.opacity = 0.02
world.publish(cfg)

# Template paddle — G4Trap general trapezoid.
# The cross-section in the local YX plane is a trapezoid: pX (wide) at local +Y,
# pLTX (narrow) at local -Y. set_rotation(0, 0, -90) maps local +Y to the global
# radial-outward direction at phi=0, so the wider face is always at larger radius
# and the narrow face points toward the barrel center.
# distribute_on_circle(align=True) adds a rotation of phi_i around Z for each copy.
paddle = GVolume("paddle")
paddle.mother = "root"
paddle.description = "Scintillator barrel paddle"
paddle.make_general_trapezoid(pZ, 0, 0, pY, pLTX, pX, 0, pY, pLTX, pX, 0)
paddle.material = "G4_PLASTIC_SC_VINYLTOLUENE"
paddle.color = "D7CCFF"
paddle.opacity = 0.8
paddle.set_rotation(0, 0, -90)   # local +Y (wide face) → radially outward
paddle.digitization = "flux"


for i, v in enumerate(paddle.distribute_on_circle(n, radius, align=True, axis='z')):
    v.set_identifier("paddle", i)
    v.publish(cfg)
