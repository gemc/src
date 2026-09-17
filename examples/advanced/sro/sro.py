#!/usr/bin/env python3
"""Four sensitive planes; the plugin's translation table assigns their electronics addresses."""

from pygemc import GVolume, autogeometry

cfg = autogeometry("examples", "sro")

world = GVolume("root")
world.description = "Simple SRO world"
world.make_box(50, 50, 60)  # Half lengths, cm.
world.material = "G4_Galactic"
world.visible = 0
world.publish(cfg)

for index, sensitive_id in enumerate((101, 102, 103, 104)):
    plane = GVolume(f"plane{sensitive_id}")
    plane.mother = "root"
    plane.description = f"Sensitive plane {sensitive_id}"
    plane.make_box(5, 5, 0.05)
    plane.set_position(0, 0, 10 * (index + 1))
    plane.material = "G4_Galactic"
    plane.color = "66aaff" if index < 2 else "ffbb66"
    plane.digitization = "example_sro"
    # Hardware mapping belongs to Digitizer::loadTTImpl, keyed by this sensitive identity.
    plane.set_identifier("id", sensitive_id)
    plane.publish(cfg)
