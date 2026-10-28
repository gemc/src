#!/usr/bin/env python3
"""Catalog of Geant4 solids exercised by the ``solid`` example.

Each entry maps a short key (also used as the geometry *variation* name) to a
``(Geant4 solid type, parameter string)`` pair. The parameter order and units
follow the Geant4 constructors documented at

    https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomSolids.html

and match what the GEMC native geometry factory parses. Dimensionless values
(plane/vertex counts, cut-plane normals, elliptical-cone slopes) carry the
``counts`` unit so the parser reads them as pure numbers.

The example builds a volume straight from these two strings, so the catalog is
the single place to add or adjust a solid.
"""

SOLIDS = {
    # --- CSG primitives ---------------------------------------------------
    'box':                 ('G4Box',             '40*mm, 40*mm, 40*mm'),
    'tube':                ('G4Tubs',            '10*mm, 40*mm, 40*mm, 0*deg, 360*deg'),
    'cut_tube':            ('G4CutTubs',         '10*mm, 40*mm, 40*mm, 0*deg, 360*deg, '
                                                 '0*counts, -0.5*counts, -0.866*counts, '
                                                 '0*counts, 0.5*counts, 0.866*counts'),
    'cone':                ('G4Cons',            '0*mm, 20*mm, 0*mm, 40*mm, 40*mm, 0*deg, 360*deg'),
    'parallelepiped':      ('G4Para',            '30*mm, 40*mm, 50*mm, 10*deg, 20*deg, 5*deg'),
    'trd':                 ('G4Trd',             '30*mm, 10*mm, 40*mm, 15*mm, 50*mm'),
    'trap_wedge':          ('G4Trap',            '60*mm, 40*mm, 50*mm, 20*mm'),
    'trap_general':        ('G4Trap',            '40*mm, 0*deg, 0*deg, 30*mm, 20*mm, 20*mm, 0*deg, '
                                                 '30*mm, 20*mm, 20*mm, 0*deg'),
    'sphere':              ('G4Sphere',          '0*mm, 40*mm, 0*deg, 360*deg, 0*deg, 180*deg'),
    'orb':                 ('G4Orb',             '40*mm'),
    'torus':               ('G4Torus',           '10*mm, 20*mm, 50*mm, 0*deg, 360*deg'),
    # --- specific solids --------------------------------------------------
    'polycone':            ('G4Polycone',        '0*deg, 360*deg, 3*counts, '
                                                 '-40*mm, 0*mm, 40*mm, '
                                                 '0*mm, 0*mm, 0*mm, '
                                                 '30*mm, 40*mm, 30*mm'),
    'generic_polycone':    ('G4GenericPolycone', '0*deg, 360*deg, 4*counts, '
                                                 '10*mm, 30*mm, 30*mm, 10*mm, '
                                                 '-20*mm, -20*mm, 20*mm, 20*mm'),
    'polyhedra':           ('G4Polyhedra',       '0*deg, 360*deg, 6*counts, 2*counts, '
                                                 '0*mm, 0*mm, 40*mm, 40*mm, -50*mm, 50*mm'),
    'elliptical_tube':     ('G4EllipticalTube',  '20*mm, 30*mm, 40*mm'),
    'ellipsoid':           ('G4Ellipsoid',       '20*mm, 30*mm, 40*mm, 0*mm, 0*mm'),
    'elliptical_cone':     ('G4EllipticalCone',  '0.5*counts, 0.5*counts, 40*mm, 25*mm'),
    'paraboloid':          ('G4Paraboloid',      '40*mm, 5*mm, 30*mm'),
    'hype':                ('G4Hype',            '10*mm, 20*mm, 10*deg, 20*deg, 40*mm'),
    'tet':                 ('G4Tet',             '0*mm, 0*mm, 0*mm, 40*mm, 0*mm, 0*mm, '
                                                 '0*mm, 40*mm, 0*mm, 0*mm, 0*mm, 40*mm'),
    # --- twisted solids ---------------------------------------------------
    'twisted_box':         ('G4TwistedBox',      '30*deg, 20*mm, 25*mm, 40*mm'),
    'twisted_trap':        ('G4TwistedTrap',     '30*deg, 20*mm, 25*mm, 20*mm, 40*mm'),
    'twisted_trap_general':('G4TwistedTrap',     '30*deg, 40*mm, 10*deg, 5*deg, 20*mm, '
                                                 '15*mm, 15*mm, 25*mm, 20*mm, 20*mm, 0*deg'),
    'twisted_trd':         ('G4TwistedTrd',      '15*mm, 25*mm, 20*mm, 30*mm, 40*mm, 30*deg'),
    'twisted_tubs':        ('G4TwistedTubs',     '30*deg, 10*mm, 20*mm, 40*mm, 90*deg'),
}
