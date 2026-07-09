#!/usr/bin/env python3
"""Boolean solid operations showcase.

Three shapes demonstrate the GEMC boolean solid operations, one per operator:

* plate_with_hole: subtraction (-) of an off-center tube from a box;
* cross:           union (+) of a bar with the same bar rotated by 90 degrees;
* lens:            intersection (*) of two spheres offset along z.

A boolean volume is defined by `solidsOpr = "first operator second"`. The two
operands are volumes published with material "Component": they contribute their
solid to the operation but are never placed. The first operand is taken at its own
origin; the second operand's position and rotation define where it sits relative to
the first. The resulting volume is then placed like any other, with its own
material, position, and rotation.
"""

from pygemc import autogeometry
from pygemc import GVolume

cfg = autogeometry("examples", "boolean_solids")

world = GVolume('root')
world.description = 'World'
world.make_box(450, 200, 200)
world.material = 'G4_AIR'
world.color = '889922'
world.style = 0
world.opacity = 0.1
world.publish(cfg)

# ---------------------------------------------------------------------------
# Subtraction: a plate with an off-center hole
# ---------------------------------------------------------------------------
plate_box = GVolume('plate_box')
plate_box.description = 'plate before the hole is punched'
plate_box.make_box(80, 80, 10)
plate_box.material = 'Component'
plate_box.publish(cfg)

# the hole position is relative to plate_box: 30 mm off-center
plate_hole = GVolume('plate_hole')
plate_hole.description = 'hole to punch through the plate'
plate_hole.make_tube(0, 40, 12, 0, 360)
plate_hole.set_position(30, 0, 0)
plate_hole.material = 'Component'
plate_hole.publish(cfg)

plate = GVolume('plate_with_hole')
plate.description = 'plate minus off-center hole'
plate.solidsOpr = 'plate_box - plate_hole'
plate.material = 'G4_Al'
plate.color = 'silver'
plate.set_position(-280, 0, 0)
plate.publish(cfg)

# ---------------------------------------------------------------------------
# Union: a cross made of one bar plus the same bar rotated by 90 degrees
# ---------------------------------------------------------------------------
bar = GVolume('bar')
bar.description = 'horizontal bar of the cross'
bar.make_box(80, 20, 20)
bar.material = 'Component'
bar.publish(cfg)

# the second operand is rotated in place: same bar, 90 degrees around z
bar_rotated = GVolume('bar_rotated')
bar_rotated.description = 'vertical bar of the cross'
bar_rotated.make_box(80, 20, 20)
bar_rotated.set_rotation(0, 0, 90)
bar_rotated.material = 'Component'
bar_rotated.publish(cfg)

cross = GVolume('cross')
cross.description = 'bar plus rotated bar'
cross.solidsOpr = 'bar + bar_rotated'
cross.material = 'G4_Cu'
cross.color = 'orange'
cross.publish(cfg)

# ---------------------------------------------------------------------------
# Intersection: a lens from two overlapping spheres
# ---------------------------------------------------------------------------
sphere_one = GVolume('sphere_one')
sphere_one.description = 'first lens sphere'
sphere_one.make_sphere(0, 100, 0, 360, 0, 180)
sphere_one.material = 'Component'
sphere_one.publish(cfg)

sphere_two = GVolume('sphere_two')
sphere_two.description = 'second lens sphere, offset along z'
sphere_two.make_sphere(0, 100, 0, 360, 0, 180)
sphere_two.set_position(0, 0, 160)
sphere_two.material = 'Component'
sphere_two.publish(cfg)

lens = GVolume('lens')
lens.description = 'intersection of the two spheres'
lens.solidsOpr = 'sphere_one * sphere_two'
lens.material = 'G4_PLEXIGLASS'
lens.color = 'lightblue'
lens.set_position(280, 0, -80)
lens.publish(cfg)
