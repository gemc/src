#!/usr/bin/env python3
from gconfiguration import autogeometry
from gvolume import GVolume

cfg = autogeometry("examples", "b2")

fNbOfChambers = 5
chamberSpacing = 800  # from chamber center to center

chamberWidth = 200.0  # width of the chambers
targetLength = 50.0  # full length of Target

trackerLength = (fNbOfChambers + 1) * chamberSpacing
worldLength = 1.2 * (2 * targetLength + trackerLength)

targetRadius = 0.5 * targetLength  # Radius of Target
targetLength = 0.5 * targetLength  # Half length of the Target
trackerSize = 0.5 * trackerLength  # Half length of the Tracker

gvolume = GVolume('root')
gvolume.description = 'World'
gvolume.make_box(worldLength * 0.5, worldLength * 0.5, worldLength * 0.5)
gvolume.material = 'G4_AIR'
gvolume.color = 'ghostwhite'
gvolume.visible = 0
gvolume.publish(cfg)

tz = -(targetLength + trackerSize)
gvolume = GVolume('target')
gvolume.description = 'Lead Target'
gvolume.make_tube(0, targetRadius, targetLength, 0, 360)
gvolume.material = 'G4_Pb'
gvolume.set_position(0, 0, tz)
gvolume.color = 'steelblue'
gvolume.publish(cfg)

gvolume = GVolume('tracker')
gvolume.description = 'Chamber Tracker Container'
gvolume.make_tube(0, trackerSize, trackerSize, 0, 360)
gvolume.material = 'G4_AIR'
gvolume.opacity = 0.1
gvolume.publish(cfg)

firstPosition = -trackerSize + chamberSpacing
firstLength = trackerLength / 10
lastLength = trackerLength
halfWidth = 0.5 * chamberWidth
rmaxFirst = 0.5 * firstLength

rmaxIncr = 0.5 * (lastLength - firstLength) / (fNbOfChambers - 1)

# loop from 0 to fNbOfChambers
for copyNo in range(fNbOfChambers):
	Zposition = firstPosition + copyNo * chamberSpacing
	rmax = rmaxFirst + copyNo * rmaxIncr
	gvolume = GVolume(f'chamber_{copyNo}')
	gvolume.mother = 'tracker'
	gvolume.description = f'Chamber Tracker {copyNo}'
	gvolume.make_tube(0, rmax, halfWidth, 0, 360)
	gvolume.material = 'G4_Xe'
	gvolume.color = 'metallic, papayawhip'
	gvolume.set_position(0, 0, Zposition)
	gvolume.digitization = 'flux'  # adds edep, pid, etc to hits
	gvolume.set_identifier('mychamber', copyNo)
	gvolume.publish(cfg)
