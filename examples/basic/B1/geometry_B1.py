from geometry_api import GVolume
import math


# These are example of methods to build a mother and daughter volume.

def build_B1(configuration):
    build_mother_volume(configuration)
    build_target(configuration)


def build_mother_volume(configuration):
    gvolume = GVolume('absorber')
    gvolume.description = 'scintillator box'
    gvolume.make_box(100.0, 100.0, 100.0)
    gvolume.material = 'carbonFiber'
    gvolume.color = '3399FF'
    gvolume.digitization = 'flux'
    gvolume.set_identifier('box', 2)  # identifier for this box
    gvolume.style = 0
    gvolume.publish(configuration)


def build_target(configuration):
    gvolume = GVolume('target')
    gvolume.description = 'epoxy target'
    gvolume.mother = 'absorber'
    gvolume.make_tube(0, 20, 40, 0, 360)
    gvolume.material = 'epoxy'
    gvolume.color = 'ff0000'
    gvolume.mfield = 'dipole'
    gvolume.publish(configuration)
