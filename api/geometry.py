# -*- coding: utf-8 -*-
# =======================================
# gemc geometry definition
#
# This file defines the GVolume class that holds the parameters needed to define a geant4 physical volume in gemc.
# The "publish" function writes out the volume parameters according to the factory.
#
# A GVolume is instantiated with these mandatory arguments:
#
# - name: The name of the volume
# - solid: a string identifying The GEANT4 solid name, as defined in the documentation:
#                https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomSolids.html
#
# - parameters: A string containing the parameters (with units) required to define the solid
#                The order of the parameters matches the GEANT4 solid constructors.
#                For example a "G4Box" type needs 3 dimensions:  pX, pY, pZ "2*cm 5*m 24*cm"
#
# - material:  A string defining the volume's material.  This can be a sci-g GMaterial name, or one from the GEANT4
#              material database:
#              https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html
#              for example, "G4_MYLAR" or "G4_PLEXIGLASS".
#
# The following GVolume parameters are optional:
#
# - mother: The name of the parent volume. Defaults to the world volume "root"
# - position:  A string giving the location of this volume relative to the GEANT4 parent solid definition.
#                   Default is "0*mm, 0*mm, 0*mm"
# - rotations: The rotation matrix of this volume relative to the GEANT4 parent solid definition.
#              Default is "0*deg, 0*deg, 0*deg"
#              The matrix can be defined by using either of two GVolume functions:
#              1. set_rotation(string):
#                 Define a single, ordered rotation.
#                  a. If the rotation string has 3 values, the rotation refers the x-, y-, and z- axes in order.
#                     For example "10*deg, 45*deg, 30*deg":
#                      rotation of 10° around x first, then 45° around y, then 30° around z.
#                  b. An additional string can be given at the end of the string to define the rotation's order.
#                     For example "10*deg, 45*deg, 30*deg, zyx":
#                      rotation of 30° around z first, then 45° around y, then 10° around x.
#              2. add_rotation(string):
#                 Adds a single rotation that refers the x-, y-, and z- axes in order. Note: add_rotation is cumulative.
#                 For example add_rotation("0*deg, 0*deg, 40*deg") follow by add_rotation("10*deg, 0*deg, 0*deg"):
#                  defines a rotation of 40° around z first then apply a rotation of 10° around x.
#
# - mfild: The name of a magnetic field file attached to the gvolume. Default is "no".
#          The field is defined in the file header. In case of a field map, the data is contained in the file itself.
#
#
# - visible: 1 if the volume should be visible when the geometry is displayed, 0 if not.
#            Default is "1".
#
# - style:  1 means display the volume as a solid,
#           0 means display as wireframe.
#           Default is "1".
#
# - color: A hexadecimal color value string, two chars for each f Red, Green, Blue (RGB) colors.  Default is "778899".
#          For example, "0000ff" is blue.
#          An optional 7th digit from 0-5 sets the transparency value where 0 is fully opaque and 5 is fully transparent
#          Example:  "0000ff4" gives the volume a mostly transparent blue color
#
# - digitization:  A string defining the name of the plugin used to digitized the hit. Default is "none".
#                   Here pre-defined plugin can be selected:
#                   - flux: each tracks leaves a hit in the volume
#                   - particleCounter: used to count particles types passing through a volume
#
#                   In addition users can define their own plugin using c++, starting from predefined templates.
#                   The plugin filename is <name>.gplugin
#
# - copyOf	       Not supported yet. Meant to make a copy of a volume
# - replicaOf	   Not supported yet. Meant to make a g4replica
# - solidsOpr	   Not supported yet. Meant to make a boolean operation between solids
# - mirror	       Not supported yet. Meant to make a g4surface
#
# - exist:  1 if the volume exists, 0 if not.  It is a way to turn off volumes.
#           This value can also be accessed in the jcard modifiers. Default is "1".
#
# - description		- A description of the volume. Default is "no description"
import sys

WILLBESET = 'notSetYet'  # for mandatory fields. Used in function check_validity
NOTAPPLICABLE = 'na'  # for optionals fields
DEFAULTMOTHER = 'root'
DEFAULTCOLOR = '778899'

from scig_sql import populate_sqlite_geometry

# GVolume class definition
class GVolume:
    def __init__(self, name):
        # mandatory fields. Checked at publish time
        self.name = name
        self.solid = WILLBESET
        self.parameters = WILLBESET
        self.material = WILLBESET

        # optional fields
        self.mother = DEFAULTMOTHER
        self.position = '0*mm, 0*mm, 0*mm'
        self.rotations = ['0*deg, 0*deg, 0*deg']
        self.mfield = NOTAPPLICABLE

        self.visible = 1  # 0 is invisible, 1 is visible
        self.style = 1    # 0 is wireframe, 1 is solid
        self.color = DEFAULTCOLOR

        self.digitization = NOTAPPLICABLE
        self.identifier = NOTAPPLICABLE

        self.copyOf = NOTAPPLICABLE
        self.replicaOf = NOTAPPLICABLE
        self.solidsOpr = NOTAPPLICABLE

        self.mirror = NOTAPPLICABLE

        self.exist = 1  # 0 does not exist, 1 exists
        self.description = NOTAPPLICABLE

    def set_rotation(self, x, y, z, lunit='deg', order=''):
        with_units = [
            f"{val}*{lunit}"
            for val in [x, y, z]
        ]
        string_with_units = ", ".join(with_units)
        if order:
            self.rotations = f"ordered: {order}, {string_with_units}"
        else:
            self.rotations = string_with_units

    def set_position(self, x, y, z, lunit='mm'):
        myposition = str(x) + '*' + lunit + ', '
        myposition += str(y) + '*' + lunit + ', '
        myposition += str(z) + '*' + lunit
        self.position = myposition

    def add_rotation(self, x, y, z, lunit='deg'):
        myrotation = str(x) + '*' + lunit + ', '
        myrotation += str(y) + '*' + lunit + ', '
        myrotation += str(z) + '*' + lunit
        self.rotations.append(' + ' + myrotation)

    def get_rotation_string(self):
        rotation_string = ''
        for r in self.rotations:
            rotation_string = rotation_string + r
        return rotation_string

    def check_validity(self):
        # need to add checking if it's operation instead
        if self.solid == WILLBESET:
            sys.exit(' Error: solid not defined for GVolume ' + str(self.name))
        if self.parameters == WILLBESET:
            sys.exit(' Error: parameters not defined for GVolume ' + str(self.name))
        if self.material == WILLBESET:
            sys.exit(' Error: material not defined for GVolume ' + str(self.name))

    # Pass a List to a Function as Multiple Arguments
    def set_identifier(self, *identifiers):
        identity_size = int(len(identifiers) / 2)
        myidentifiers = ''
        # looping over all pairs. Last one will not have the final comma
        for i in range(identity_size):
            idname = identifiers[2 * i]
            idtag = identifiers[2 * i + 1]
            myidentifiers += str(idname) + ': '
            if i == identity_size - 1:
                myidentifiers += str(idtag)
            else:
                myidentifiers += str(idtag) + ', '

        self.identifier = myidentifiers

    def publish(self, configuration):
        self.check_validity()
        # TEXT factory
        if configuration.factory == 'TEXT':
            file_name = configuration.geoFileName
            configuration.nvolumes += 1
            with open(file_name, 'a+') as dn:
                lstr = ' ' \
                       + f'{self.name} | ' \
                       + f'{self.solid} | ' \
                       + f'{self.parameters} | ' \
                       + f'{self.material} | ' \
                       + f'{self.mother} | ' \
                       + f'{self.position} | ' \
                       + f'{self.get_rotation_string()} | ' \
                       + f'{self.mfield} | ' \
                       + f'{self.visible} | ' \
                       + f'{self.style} | ' \
                       + f'{self.color} | ' \
                       + f'{self.digitization} | ' \
                       + f'{self.identifier} | ' \
                       + f'{self.copyOf} | ' \
                       + f'{self.replicaOf} | ' \
                       + f'{self.solidsOpr} | ' \
                       + f'{self.mirror} | ' \
                       + f'{self.exist} | ' \
                       + f'{self.description} |\n'
                dn.write(lstr)
        # SQLITE factory
        elif configuration.factory == 'SQLITE':
            configuration.nvolumes += 1
            self.rotations = self.get_rotation_string()
            populate_sqlite_geometry(self, configuration)

    # Functions to build geant4 solids

    def make_box(self, dx, dy, dz, lunit='mm'):
        """
        make_box(dx, dy, dz, lunit='mm')

        Creates a geant4 Box

        Parameters
        ----------

        dx : half length in x
        dy : half length in y
        dz : half length in z
        lunit: length unit (optional; default: mm)

        Example
        -------

        Creates a box with dx=30mm, dy=40mm, dz=60mm:

        > make_box(30, 40, 60)

        To print the corresponding code:

        > scigTemplate.py -gv G4Box -gvp '30, 40, 60' -silent

        To print the generic code:

        > scigTemplate.py -gv G4Box
        """

        self.solid = WILLBESET
        self.solid = 'G4Box'
        my_lengths: str = str(dx) + '*' + lunit + ', '
        my_lengths += str(dy) + '*' + lunit + ', '
        my_lengths += str(dz) + '*' + lunit
        self.parameters = my_lengths

    # Cylindrical Section or Tube
    def make_tube(self, rin, rout, length, phistart, phitotal, lunit1='mm', lunit2='deg'):
        """
        make_tube(rin, rout, length, phistart, phitotal, lunit1='mm', lunit2='deg')

        Creates a geant4 Cylindrical Section or Tube

        Parameters
        ----------

        rin : inner radius
        rout : outer radius
        length : tube half length in z
        phistart : starting phi angle
        phitotal : total phi angle
        lunit1: length unit (optional; default: mm)
        lunit2: angle unit (optional; default: deg)

        Example
        -------

        Creates a tube with rin=10mm, rout=15mm, length=20mm, phistart=0deg, phitotal=90deg:

        > make_tube(10, 15, 20, 0, 90)

        To print the corresponding code:

        > scigTemplate.py -gv G4Tubs -gvp '10, 15, 20, 0, 90' -silent

        To print the generic code:

        > scigTemplate.py -gv G4Tubs

        """

        self.solid = 'G4Tubs'
        my_dims: str = str(rin) + '*' + lunit1 + ', '
        my_dims += str(rout) + '*' + lunit1 + ', '
        my_dims += str(length) + '*' + lunit1 + ', '
        my_dims += str(phistart) + '*' + lunit2 + ', '
        my_dims += str(phitotal) + '*' + lunit2
        self.parameters = my_dims

    # Cylindrical Cut Section or Cut Tube
    # def make_cut_tube()

    # Cone or Conical section
    def make_cone(self, rin1, rout1, rin2, rout2, length, phi_start, phi_total, lunit1='mm', lunit2='deg'):
        """
        make_cone(rin1, rout1, rin2, rout2, length, phi_start, phi_total, lunit1='mm', lunit2='deg')

        Creates a geant4 Cone or Conical section

        Parameters
        ----------

        rin1 : inner radius at -dz
        rout1 : outer radius at -dz
        rin2 : inner radius at +dz
        rout2 : outer radius at +dz
        length : cone half length in z
        phi_start : starting phi angle
        phi_total : total phi angle
        lunit1: length unit (optional; default: mm)
        lunit2: angle unit (optional; default: deg)

        Example
        -------

        Creates a cone with rin1=5mm, rout1=10mm, rin2=20mm, rout2=25mm, length=40mm, phi_start=0deg, phi_total=270deg:

        > make_cone(5, 10, 20, 25, 40, 0, 270)

        To print the corresponding code:

        > scigTemplate.py -gv G4Cons -gvp '5, 10, 20, 25, 40, 0, 270' -silent

        To print the generic code:

        > scigTemplate.py -gv G4Cons

        """

        self.solid = 'G4Cons'
        mydims = str(rin1) + '*' + lunit1 + ', ' + str(rout1) + '*' + lunit1 + ', ' \
                 + str(rin2) + '*' + lunit1 + ', ' \
                 + str(rout2) + '*' + lunit1 + ', ' \
                 + str(length) + '*' + lunit1 + ', ' \
                 + str(phi_start) + '*' + lunit2 + ', ' \
                 + str(phi_total) + '*' + lunit2
        self.parameters = mydims

    # Trapezoid
    def make_trapezoid(self, dx1, dx2, dy1, dy2, z, lunit='mm'):
        """
        make_trapezoid(dx1, dx2, dy1, dy2, z, lunit='mm')

        Creates a geant4 Trapezoid

        Parameters
        ----------

        dx1 : half length in x at -dz
        dx2 : half-length in x at +dz
        dy1 : half-length in y at -dz
        dy2 : half-length in y at +dz
        z : half-length in z
        lunit: length unit (optional; default: mm)

        Example
        -------

        Creates a trapezoid with dx1=30mm, dx2=10mm, dy1=40mm, dy2=15mm, z=60mm:

        > make_trapezoid(30, 10, 40, 15, 60)

        To print the corresponding code:

        > scigTemplate.py -gv G4Trd -gvp '30, 10, 40, 15, 60' -silent

        To print the generic code:

        > scigTemplate.py -gv G4Trd


        """

        self.solid = "G4Trd"
        with_units = [
            f"{val}*{lunit}"
            for val in [dx1, dx2, dy1, dy2, z]
        ]
        self.parameters = ", ".join(with_units)

    def make_trap_from_angular_wedges(self, pz, py, px, pltx, unit='mm'):
        """
        make_trap_from_angular_wedges(pz, py, px, pltx, lunit1='mm')

        Creates a geant4 Generic Trapezoid: right Angular Wedge (4 parameters)

        Parameters
        ----------

        pz : Length along Z
        py : Length along Y
        px : Length along X at the wider side
        pltx : Length along X at the narrower side (plTX<=pX)
        unit: length unit (optional; default: mm)

        Example
        -------

        TO VERIFY:
        Creates a trapezoid with pz=30mm, py=40mm, px=50mm, pltx=20mm:

        > make_trap_from_angular_wedges(30, 40, 50, 20)

        To print the corresponding code:

        > scigTemplate.py -gv G4TrapRAW -gvp '30, 40, 50, 20' -silent

        To print the generic code:

        > scigTemplate.py -gv G4TrapRAW

        """

        self.solid = "G4Trap"
        with_units = [
            f"{pz}*{unit}",
            f"{py}*{unit}",
            f"{px}*{unit}",
            f"{pltx}*{unit}"
        ]
        self.parameters = ", ".join(with_units)

    def make_general_trapezoid(self, pDz, pTheta, pPhi, pDy1, pDx1, pDx2, pAlp1, pDy2, pDx3, pDx4, pAlp2,
                               lunit1='mm', lunit2='deg'):
        """

        make_general_trapezoid(pDz, pTheta, pPhi, pDy1, pDx1, pDx2, pAlp1, pDy2, pDx3, pDx4, pAlp2, lunit1='mm', lunit2='deg')

        Creates a geant4 Generic Trapezoid: general trapezoid (11 parameters)

        Parameters
        ----------

        pDz: Half Z length - distance from the origin to the bases
        pTheta: Polar angle of the line joining the centres of the bases at -/+pDz
        pPhi: Azimuthal angle of the line joining the centre of the base at -pDz to the centre of the base at +pDz
        pDy1: Half Y length of the base at -pDz
        pDy2: Half Y length of the base at +pDz
        pDx1: Half X length at smaller Y of the base at -pDz
        pDx2: Half X length at bigger Y of the base at -pDz
        pDx3: Half X length at smaller Y of the base at +pDz
        pDx4: Half X length at bigger y of the base at +pDz
        pAlp1: Angle between the Y-axis and the centre line of the base at -pDz (lower endcap)
        pAlp2: Angle between the Y-axis and the centre line of the base at +pDz (upper endcap)
        lunit1: length unit (optional; default: mm)
        lunit2: angle unit (optional; default: deg)

        Example
        -------

        Creates a trapezoid with pDz=30mm, pTheta=40deg, pPhi=50deg, pDy1=60mm, pDx1=70mm, pDx2=80mm, pAlp1=90deg, pDy2=100mm, pDx3=110mm, pDx4=120mm, pAlp2=130deg:

        > make_general_trapezoid(30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130)

        To print the corresponding code:

        > scigTemplate.py -gv G4TrapG -gvp '30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130' -silent

        To print the generic code:

        > scigTemplate.py -gv G4TrapG

        """

        self.solid = "G4Trap"
        with_units = [
            f"{pDz}*{lunit1}",
            f"{pTheta}*{lunit2}",
            f"{pPhi}*{lunit2}",
            f"{pDy1}*{lunit1}",
            f"{pDx1}*{lunit1}",
            f"{pDx2}*{lunit1}",
            f"{pAlp1}*{lunit2}",
            f"{pDy2}*{lunit1}",
            f"{pDx3}*{lunit1}",
            f"{pDx4}*{lunit1}",
            f"{pAlp2}*{lunit2}"
        ]
        self.parameters = ", ".join(with_units)

    def make_trap_from_vertices(self,
                                v1x, v1y, v1z,
                                v2x, v2y, v2z,
                                v3x, v3y, v3z,
                                v4x, v4y, v4z,
                                v5x, v5y, v5z,
                                v6x, v6y, v6z,
                                v7x, v7y, v7z,
                                v8x, v8y, v8z,
                                lunit1='mm'):
        """

        make_trap_from_vertices(v1x, v1y, v1z, v2x, v2y, v2z, v3x, v3y, v3z, v4x, v4y, v4z, v5x, v5y, v5z, v6x, v6y,
        v6z, v7x, v7y, v7z, v8x, v8y, v8z, lunit1='mm')

        Creates a geant4 Generic Trapezoid: from eight points (24 parameters)

        Parameters
        ----------

        v1x, v1y, v1z: Coordinates of the first vertex
        v2x, v2y, v2z: Coordinates of the second vertex
        v3x, v3y, v3z: Coordinates of the third vertex
        v4x, v4y, v4z: Coordinates of the fourth vertex
        v5x, v5y, v5z: Coordinates of the fifth vertex
        v6x, v6y, v6z: Coordinates of the sixth vertex
        v7x, v7y, v7z: Coordinates of the seventh vertex
        v8x, v8y, v8z: Coordinates of the eighth vertex
        lunit1: length unit (optional; default: mm)

        v1, v2 | Edge with smaller Y of the base at -z
        v3, v4 | Edge with bigger Y of the base at -z
        v5, v6 | Edge with smaller Y of the base at +z
        v7, v8 | Edge with bigger Y of the base at +z

        Example
        -------

        MISSING, build one

        """

        self.solid = "G4Trap"
        with_units = [
            f"{v1x}*{lunit1}",
            f"{v1y}*{lunit1}",
            f"{v1z}*{lunit1}",
            f"{v2x}*{lunit1}",
            f"{v2y}*{lunit1}",
            f"{v2z}*{lunit1}",
            f"{v3x}*{lunit1}",
            f"{v3y}*{lunit1}",
            f"{v3z}*{lunit1}",
            f"{v4x}*{lunit1}",
            f"{v4y}*{lunit1}",
            f"{v4z}*{lunit1}",
            f"{v5x}*{lunit1}",
            f"{v5y}*{lunit1}",
            f"{v5z}*{lunit1}",
            f"{v6x}*{lunit1}",
            f"{v6y}*{lunit1}",
            f"{v6z}*{lunit1}",
            f"{v7x}*{lunit1}",
            f"{v7y}*{lunit1}",
            f"{v7z}*{lunit1}",
            f"{v8x}*{lunit1}",
            f"{v8y}*{lunit1}",
            f"{v8z}*{lunit1}"
        ]
        self.parameters = ", ".join(with_units)

    # Generic Trapezoid: will call the G4Trap constructor based on the number of parameters
    # - for a Right Angular Wedge (4 parameters)
    # - for a general trapezoid (11 parameters)
    # - from eight points (8 parameters)
    def make_trap(self, params, lunit1='mm', lunit2='deg'):
        """

        make_trap(params, lunit1='mm', lunit2='deg')

        Creates a geant4 Generic Trapezoid: will call the G4Trap constructor based on the number of parameters

        Parameters
        ----------

        params: list of parameters
        lunit1: length unit (optional; default: mm)
        lunit2: angle unit (optional; default: deg)

        Example:
        --------

        MISSING, build one

        """

        if len(params) == 4:
            self.make_trap_from_angular_wedges(self, *params, lunit1)
        elif len(params) == 11:
            self.make_general_trapezoid(params[0], params[1], params[2], params[3], params[4],
                                        params[5], params[6], params[7], params[8], params[9],
                                        params[10], lunit1, lunit2)
        elif len(params) == 24:
            self.make_trap_from_vertices(self, *params, lunit1)
        else:
            sys.exit(' Error: the G4Trap eight points constructor parameter must be an array with 24 points')

    def make_sphere(self, rmin, rmax, sphi, dphi, stheta, dtheta, lunit1='mm', lunit2='deg'):
        """
        make_sphere(rmin, rmax, sphi, dphi, stheta, dtheta, lunit1='mm', lunit2='deg')

        Creates a geant4 Sphere or Spherical Shell Section

        Parameters
        ----------

        rmin: inner radius
        rmax: outer radius
        sphi: starting phi angle
        dphi: delta phi angle
        stheta: starting theta angle
        dtheta: delta theta angle
        lunit1: length unit (optional; default: mm)
        lunit2: angle unit (optional; default: deg)

        """
        self.solid = WILLBESET
        self.solid = 'G4Sphere'
        dimensions = str(rmin) + '*' + lunit1 + ', ' \
                     + str(rmax) + '*' + lunit1 + ', ' \
                     + str(sphi) + '*' + lunit2 + ', ' + str(dphi) + '*' + lunit2 + ', ' \
                     + str(stheta) + '*' + lunit2 + ', ' + str(dtheta) + '*' + lunit2
        self.parameters = dimensions

    # "G4Orb": "Full Solid Sphere",
    # "G4Torus": "Torus",

    # in polycone the zplane and radius order are swapped w.r.t. gemc2 implementation
    # in order to match the geant4 constructor
    def make_polycone(self, phiStart, phiTotal, zplane, iradius, oradius, lunit1='mm', lunit2='deg'):
        """
        make_polycone(phiStart, phiTotal, zplane, iradius, oradius, lunit1='mm', lunit2='deg')

        Creates a geant4 Polycone

        Parameters
        ----------

        phiStart: starting phi angle
        phiTotal: total phi angle
        zplane: list of z coordinates
        iradius: list of inner radii
        oradius: list of outer radii
        lunit1: length unit (optional; default: mm)
        lunit2: angle unit (optional; default: deg)

        Example
        -------

        MISSING, build one

        """

        nplanes = len(zplane)
        if not len(iradius) == nplanes and not len(oradius) == nplanes:
            sys.exit(
                ' Error: the G4Polycone array lengths do not match: zplane=' + str(len(zplane)) + ', iradius=' + str(
                    len(iradius)) + ', oradius=' + str(len(oradius)))

        self.solid = 'G4Polycone'
        mylengths = ' '
        for ele in zplane:
            mylengths += str(ele) + '*' + lunit1 + ', '
        for ele in iradius:
            mylengths += str(ele) + '*' + lunit1 + ', '
        for ele in oradius[:-1]:
            mylengths += str(ele) + '*' + lunit1 + ', '

        # last element w/o the extra comment
        mylengths += str(oradius[-1]) + '*' + lunit1
        self.parameters = f'{phiStart}*{lunit2}, {phiTotal}*{lunit2}, {nplanes}, {mylengths}'
