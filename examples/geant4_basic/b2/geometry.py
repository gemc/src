from geometry_api import GVolume
import math


# These are example of methods to build a mother and daughter volume.

def build_B1_geometry(configuration):
	build_mother_volume(configuration)
	build_target(configuration)


def build_mother_volume(configuration):
	gvolume = GVolume('babsorber')
	gvolume.description = 'scintillator box'
	gvolume.make_box(100.0, 100.0, 100.0)
	gvolume.material = 'bcarbonFiber'
	gvolume.color = 'ff8833'
	gvolume.digitization = 'flux'
	gvolume.set_identifier('box', 2)  # identifier for this box
	gvolume.style = 0
	gvolume.mfield = 'dipole'
	gvolume.publish(configuration)


def build_target(configuration):
	gvolume = GVolume('btarget')
	gvolume.description = 'epoxy target'
	gvolume.mother = 'babsorber'
	gvolume.make_tube(0, 20, 40, 0, 360)
	gvolume.material = 'G4_H'
	gvolume.color = '00ff00'
	gvolume.publish(configuration)
