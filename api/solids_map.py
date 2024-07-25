#!/usr/bin/env python3

g4htmlImages = 'https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/_images/'

# AVAILABLE_SOLIDS_MAP
# the key is the name of the geant4 solid. The value is a list of 3 elements:
# - the geant4 description of the solid
# - the name of the sci-g api function that creates the solid
# - the geant4 solid image link
# the commented out names are the ones not implemented yet

AVAILABLE_SOLIDS_MAP = {
    "G4Box": ["Simple Box",
              "make_box",
              "aBox.jpg"],
    "G4Tubs": ["Cylindrical Section or Tube",
               "make_tube",
               "aTubs.jpg"],
    # "G4CutTubs": "Cylindrical Cut Section or Cut Tube",
    "G4Cons": ["Cone or Conical section",
               "make_cone",
               "aCons.jpg"],
    # "G4Para": "Parallelepiped",
    "G4Trd": ["Trapezoid",
              "make_trapezoid",
              "aTrd.jpg"],
    "G4TrapRAW": ["Generic Trapezoid: right Angular Wedge (4 parameters)",
                  "make_trap_from_angular_wedges",
                  "aTrap.jpg"],
    "G4TrapG": ["Generic Trapezoid: general trapezoid (11 parameters)",
                "make_general_trapezoid",
                "wTrap.jpg"],
    "G4Trap8": ["Generic Trapezoid: from eight points (24 parameters)",
                "make_trap_from_vertices",
                "aTrap.jpg"],
    "G4Trap": ["Generic Trapezoid: will call the G4Trap constructor based on the number of parameters",
               "make_trap",
               "aTrap.jpg"],
    "G4Sphere": ["Sphere or Spherical Shell Section",
                 "make_sphere",
                 "aSphere.jpg"],
    # "G4Orb": "Full Solid Sphere",
    # "G4Torus": "Torus",
    "G4Polycone": ["Polycons",
                   "make_polycone",
                   "aBREPSolidPCone.jpg"],
    # "G4GenericPolycone": "Generic Polycone",
    # "G4Polyhedra": "Polyhedra",
    # "G4EllipticalTube": "Tube with an elliptical cross-section",
    # "G4Ellipsoid": "General Ellipsoid",
    # "G4EllipticalCone": "Cone with Elliptical Cross-Section",
    # "G4Paraboloid": "Paraboloid, a solid with parabolic profile",
    # "G4Hype": "Tube with Hyperbolic Profile",
    # "G4Tet": "Tetrahedra",
    # "G4ExtrudedSolid": "Extruded Polygon",
    # "G4TwistedBox": "Box Twisted",
    # "G4TwistedTrap": "Trapezoid Twisted along One Axis",
    # "G4TwistedTrd": "Twisted Trapezoid with X and Y dimensions varying along Z",
    # "G4GenericTrap": "Generic trapezoid with optionally collapsing vertices",
    # "G4TwistedTubs": "Tube Section Twisted along Its Axis"
}
