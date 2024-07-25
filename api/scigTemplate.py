#!/usr/bin/env python3

# imports: do not edit these lines
import argparse
import logging
import os

# from gemc_api_geometry import *
from solids_map import AVAILABLE_SOLIDS_MAP

_logger = logging.getLogger("sci-g")

NGIVEN: str = 'NOTGIVEN'
NGIVENS: [str] = ['NOTGIVEN']

# Purposes:
# 1. write a geometry/material/mirror template file, using the system name and optional variation
# 2. print on screen geometry/material python snippets
# 3. list solid types
# 4. print on screen html help for all solid types


def main():
    logging.basicConfig(level=logging.DEBUG)

    # Provides the -h, --help message
    desc_str = "   SCI-G template creator\n"
    parser = argparse.ArgumentParser(description=desc_str)

    # file writers
    parser.add_argument('-s', metavar='system', action='store', type=str,
                        help='write geometry / materials templates for system name', default=NGIVEN)
    parser.add_argument('-v', metavar='variation', action='store', type=str,
                        help='sets system variation(s). If not provided, \'default\' will be used ', nargs='*',
                        default=['default'])

    # code snippets loggers: volume
    parser.add_argument('-silent', action='store_true', help='do not print the commented line of code',
                        default=False)
    parser.add_argument('-sl',     action='store_true', help='show available solids list')  # and geant4 link
    parser.add_argument('-gv',     metavar='volume', action='store', type=str,
                        help="show on screen sci-g code for selected geant4 volume type. "
                             "Use ' -sl ' to list the available types.",
                        default=NGIVEN)
    parser.add_argument('-gvp', metavar='volume parameters', action='store', type=str,
                        help="assign parameters to the geant4 volume type selected with the gv option",
                        default=NGIVENS)
    parser.add_argument('-gmatFM', metavar='material', action='store', type=str,
                        help='show on screen sci-g code for a material defined using fractional masses',
                        default=NGIVEN)
    parser.add_argument('-gmatNA', metavar='material', action='store', type=str,
                        help='show on screen sci-g code for a material defined using number of atoms',
                        default=NGIVEN)

    args = parser.parse_args()
    # print(vars(args))

    if args.s != NGIVEN:
        write_templates(args.s, args.v)

    if args.gv != NGIVEN:
        silent: bool = args.silent
        volume_type: str = args.gv
        if args.gvp != NGIVENS:
            pars = args.gvp.split()
            pars = [p.replace(',', '') for p in pars]
            log_gvolume(silent, volume_type, pars)
        else:
            log_gvolume(silent, volume_type)

    if args.sl:
        print_all_g4solids()

    # if no argument is given print help
    if len(sys.argv) == 1:
        parser.print_help(sys.stderr)
        print()
        sys.exit(1)


def ask_to_overwrite_file(path):
    if os.path.exists(path):
        check_with_user = input('File already exist. Ok to overwrite? y/n ')
        if check_with_user == "y":
            print(f'Overwriting file {path}')
        else:
            print('Stopping execution')
            exit()

def write_templates(system, variations):

    print()
    print(f' Writing files system template >{system}< using variations >{variations}<:')
    print()
    print(f'  - {system}.py')
    print(f'  - geometry.py')
    print(f'  - materials.py')
    print(f'  - README.md')
    print()
    print(f'  - Variations defined in {system}.py:')
    for v in variations:
        print(f'    * {v}')
    print()

    # create directory if not existing
    if not os.path.exists(system):
        os.makedirs(system)

    system_script = system + '/' + system + '.py'
    geo_script    = system + '/geometry.py'
    mat_script    = system + '/materials.py'
    jcard         = system + '/' + system + '.jcard'
    readme        = system + '/README.md'


    ask_to_overwrite_file(system_script)
    with open(f'{system_script}', 'w') as ps:
        ps.write('#!/usr/bin/env python3\n\n')
        ps.write('# python:\n')
        ps.write('import sys, os, argparse\n')
        ps.write('import logging\n')
        ps.write('import subprocess\n\n')
        ps.write('# sci-g:\n')
        ps.write('from gemc_api_utils import GConfiguration\n\n')
        ps.write(f'# {system}:\n')
        ps.write('from materials import define_materials\n')
        ps.write(f'from geometry import build_{system}\n\n')

        ps.write(f'_logger = logging.getLogger("{system}")\n\n')
        ps.write('VARIATIONS = {\n')
        for v in variations:
            ps.write(f'    \"{v}",\n')
        ps.write('}\n\n')

        ps.write('def main():\n')
        ps.write('	logging.basicConfig(level=logging.DEBUG)\n\n')
        ps.write('	# Provides the -h, --help message\n')
        ps.write(f'	desc_str = "   Will create the {system}')
        ps.write(' system\\n"\n')
        ps.write('	parser = argparse.ArgumentParser(description=desc_str)\n')
        ps.write('	args = parser.parse_args()\n\n')
        ps.write('	for variation in VARIATIONS:\n\n')
        ps.write(f'		_logger.info(f"Building {system} volumes for variation')
        ps.write(' {variation}")\n')
        ps.write('		# Define GConfiguration name, factory and description.\n')
        ps.write(f'		configuration = GConfiguration(\'{system}\', \'TEXT\', \'The {system} system\')\n')
        ps.write('		configuration.setVariation(variation)\n\n')
        ps.write('		# define materials\n')
        ps.write('		configuration.init_mats_file()\n')
        ps.write('		define_materials(configuration)\n\n')
        ps.write('		# build geometry\n')
        ps.write('		configuration.init_geom_file()\n')
        ps.write(f'		build_{system}(configuration)\n\n')
        ps.write('		# print out the GConfiguration\n')
        ps.write('		configuration.printC()\n\n\n')
        ps.write('if __name__ == "__main__":\n')
        ps.write('	main()\n\n\n')
    # change permission
    os.chmod(system_script, 0o755)

    ask_to_overwrite_file(mat_script)
    with open(f'{mat_script}', 'w') as pm:
        pm.write('from gemc_api_materials import GMaterial\n\n')
        pm.write('def define_materials(configuration):\n\n')
        pm.write('# example of material: epoxy glue, defined with number of atoms\n')
        pm.write('	gmaterial = GMaterial("epoxy")\n')
        pm.write('	gmaterial.description = "epoxy glue 1.16 g/cm3"\n')
        pm.write('	gmaterial.density = 1.16\n')
        pm.write('	gmaterial.addNAtoms("H",  32)\n')
        pm.write('	gmaterial.addNAtoms("N",   2)\n')
        pm.write('	gmaterial.addNAtoms("O",   4)\n')
        pm.write('	gmaterial.addNAtoms("C",  15)\n')
        pm.write('	gmaterial.publish(configuration)\n\n')
        pm.write('# example of material: carbon fiber, defined using the fractional mass\n')
        pm.write('	gmaterial = GMaterial("carbonFiber")\n')
        pm.write('	gmaterial.description = "carbon fiber - 1.75g/cm3"\n')
        pm.write('	gmaterial.density = 1.75\n')
        pm.write('	gmaterial.addMaterialWithFractionalMass("G4_C",  0.745)\n')
        pm.write('	gmaterial.addMaterialWithFractionalMass("epoxy", 0.255)\n')
        pm.write('	gmaterial.publish(configuration)\n\n\n\n')

    ask_to_overwrite_file(geo_script)
    with open(f'{geo_script}', 'w') as pg:
        pg.write('from gemc_api_geometry import GVolume\n')
        pg.write('import math\n\n')
        pg.write('# These are example of methods to build a mother and daughter volume.\n\n')
        pg.write(f'def build_{system}(configuration):\n')
        pg.write('	build_mother_volume(configuration)\n')
        pg.write('	build_target(configuration)\n\n')
        pg.write('def build_mother_volume(configuration):\n')
        pg.write('	gvolume = GVolume(\'absorber\')\n')
        pg.write('	gvolume.description = \'scintillator box\'\n')
        pg.write('	gvolume.make_box(100.0, 100.0, 100.0)\n')
        pg.write('	gvolume.material    = \'carbonFiber\'\n')
        pg.write('	gvolume.color       = \'3399FF\'\n')
        pg.write('	gvolume.digitization = \'flux\'\n')
        pg.write('	gvolume.set_identifier(\'box\', 2)  # identifier for this box\n')

        pg.write('	gvolume.style       = 0\n')
        pg.write('	gvolume.publish(configuration)\n\n')
        pg.write('def build_target(configuration):\n')
        pg.write('	gvolume = GVolume(\'target\')\n')
        pg.write('	gvolume.description = \'epoxy target\'\n')
        pg.write('	gvolume.mother    = \'absorber\'\n')
        pg.write('	gvolume.make_tube(0, 20, 40, 0, 360)\n')
        pg.write('	gvolume.material    = \'epoxy\'\n')
        pg.write('	gvolume.color       = \'ff0000\'\n')
        pg.write('	gvolume.publish(configuration)\n\n\n\n')

    ask_to_overwrite_file(jcard)
    with open(f'{jcard}', 'w') as pj:
        pj.write('{\n')
        pj.write('	# no nthreads specified: runs on all available threads\n')
        pj.write('	# uncomment this line to specify number of threads\n')
        pj.write('	# "nthreads": 4,\n\n')
        pj.write('	# verbosities\n')
        pj.write('	"verbosity": 1,\n')
        pj.write('	"g4displayv": 2,\n')
        pj.write('	"gsystemv": 1,\n')
        pj.write('	"g4systemv": 2,\n\n')
        pj.write(f'	# the {system} system\n')
        pj.write('	"+gsystem": [\n')
        pj.write('		{\n')
        pj.write(f'			"system":   "./{system}",\n')
        pj.write('			"factory": "text",\n')
        pj.write('			"variation": "default"\n')
        pj.write('		}\n')
        pj.write('	],\n\n')
        pj.write('	"+goutput": [\n')
        pj.write('		{\n')
        pj.write('			"format": "ROOT",\n')
        pj.write('			"name": "events.root",\n')
        pj.write('			"type": "event"\n')
        pj.write('		},\n')
        pj.write('		{\n')
        pj.write('			"format": "TEXT",\n')
        pj.write('			"name": "events.txt",\n')
        pj.write('			"type": "event"\n')
        pj.write('		}\n')
        pj.write('	],\n\n')
        pj.write('	"n": 200,\n')
        pj.write('	"+gparticle": [\n')
        pj.write(
            '		{ "pname": "pi0", "multiplicity": 1,  "p": 300,  "theta": 20.0, "delta_phi": 180.0, "vz": -20.0}\n')
        pj.write('	]\n')

        pj.write('}\n\n')

    ask_to_overwrite_file(readme)
    with open(f'{readme}', 'w') as rm:
        gemc='[GEMC: Monte Carlo Particles and Hardware Simulator](https://gemc.github.io/home/)'
        rm.write('\n')
        """write 20 spaces then system name then 20 spaces"""
        rm.write(f'|{" " * 20}{gemc}{" " * 20}|\n')
        """write as many dashes as the length of the system name plus 40"""
        rm.write('|:' + '-' * (len(gemc) + 38) + ':|\n')
        """center system and description"""
        left_right_space = int((40 - len(gemc) - 12) / 2)
        rm.write(f'|{" " * left_right_space}Name and Summary Description{" " * left_right_space}|\n\n\n')
        rm.write('### Description\n\n')
        rm.write('### Usage\n\n')
        rm.write('- #### Building the detector\n\n')
        rm.write('- #### Running gemc\n\n')
        rm.write('- #### Examples\n\n')
        rm.write('### Output\n\n')
        rm.write('### Notes\n\n')
        rm.write('### Author(s)\n\n')
        rm.write('### References\n\n')


def check_units(unit_string) -> str:
    """check if units are valid and return the unit string"""
    if unit_string in ['mm', 'cm', 'm', 'rad', 'deg', 'mrad', 'urad', 'ns', 's', 'MeV', 'GeV', 'keV', 'eV']:
        return unit_string


def log_gvolume(silent, volume_type, parameters: [str] = None):
    volume_definitions = ['gvolume = GVolume(\"volume name\")']
    if volume_type == 'G4Box':
        if parameters is None:
            volume_definitions.append('gvolume.make_box(dx, dy, dz) # default units: mm.')
        elif len(parameters) == 3:
            volume_definitions.append(
                f'gvolume.make_box({parameters[0]}, {parameters[1]}, {parameters[2]}) # default units: mm.')
        elif len(parameters) == 4:
            unit = check_units(parameters[3])
            volume_definitions.append(
                f'gvolume.make_box({parameters[0]}, {parameters[1]}, {parameters[2]}, \'{unit}\')')

    elif volume_type == 'G4Tubs':
        if parameters is None:
            volume_definitions.append(
                'gvolume.make_tube(rin, rout, length, phiStart, totalPhi) # default units: mm and degrees')
        elif len(parameters) == 5:
            volume_definitions.append(
                f'gvolume.make_tube({parameters[0]}, {parameters[1]}, {parameters[2]}, {parameters[3]}, '
                f'{parameters[4]}) # default units: mm and degrees')
        elif len(parameters) == 6:
            unit = check_units(parameters[5])
            volume_definitions.append(
                f'gvolume.make_tube({parameters[0]}, {parameters[1]}, {parameters[2]}, {parameters[3]}, '
                f'{parameters[4]}, \'{unit}\')')
        elif len(parameters) == 7:
            unit = check_units(parameters[5])
            unit2 = check_units(parameters[6])
            volume_definitions.append(
                f'gvolume.make_tube({parameters[0]}, {parameters[1]}, {parameters[2]}, {parameters[3]}, '
                f'{parameters[4]}, \'{unit}\', \'{unit2}\')')

    elif volume_type == 'G4Cons':
        if parameters == NGIVENS:
            volume_definitions.append(
                'gvolume.make_cons(rin1, rout1, rin2, rout2, length, phiStart, totalPhi) # default units: mm and '
                'degrees')
        elif len(parameters) == 7:
            volume_definitions.append(
                f'gvolume.make_cons({parameters[0]}, {parameters[1]}, {parameters[2]}, {parameters[3]}, '
                f'{parameters[4]}, {parameters[5]}, {parameters[6]}) # default units: mm and degrees')
        elif len(parameters) == 8:
            unit = check_units(parameters[7])
            volume_definitions.append(
                f'gvolume.make_cons({parameters[0]}, {parameters[1]}, {parameters[2]}, {parameters[3]}, '
                f'{parameters[4]}, {parameters[5]}, {parameters[6]}, \'{unit}\')')
        elif len(parameters) == 9:
            unit = check_units(parameters[7])
            unit2 = check_units(parameters[8])
            volume_definitions.append(
                f'gvolume.make_cons({parameters[0]}, {parameters[1]}, {parameters[2]}, {parameters[3]}, '
                f'{parameters[4]}, {parameters[5]}, {parameters[6]}, \'{unit}\', \'{unit2}\')')

    elif volume_type == 'G4Trd':
        if parameters is None:
            volume_definitions.append(
                'gvolume.make_trd(dx1, dx2, dy1, dy2, dz) # default units: mm.')
        elif len(parameters) == 5:
            volume_definitions.append(
                f'gvolume.make_trd({parameters[0]}, {parameters[1]}, {parameters[2]}, {parameters[3]}, '
                f'{parameters[4]}) # default units: mm.')
        elif len(parameters) == 6:
            unit = check_units(parameters[5])
            volume_definitions.append(
                f'gvolume.make_trd({parameters[0]}, {parameters[1]}, {parameters[2]}, {parameters[3]}, '
                f'{parameters[4]}, \'{unit}\')')

    elif volume_type == 'G4TrapRAW':
        if parameters is None:
            volume_definitions.append(
                'gvolume.make_trap_from_angular_wedges(dx1, dx2, dy1, dy2, dz, theta, phi) # default units: mm.')
        elif len(parameters) == 7:
            volume_definitions.append(
                f'gvolume.make_trap_from_angular_wedges({parameters[0]}, {parameters[1]}, {parameters[2]}, '
                f'{parameters[3]}, {parameters[4]}, {parameters[5]}, {parameters[6]}) # default units: mm.')
        elif len(parameters) == 8:
            unit = check_units(parameters[7])
            volume_definitions.append(
                f'gvolume.make_trap_from_angular_wedges({parameters[0]}, {parameters[1]}, {parameters[2]}, '
                f'{parameters[3]}, {parameters[4]}, {parameters[5]}, {parameters[6]}, \'{unit}\')')

    elif volume_type == 'G4TrapG':
        if parameters is None:
            volume_definitions.append(
                'gvolume.make_general_trapezoid(pDz, pTheta, pPhi, pDy1, pDx1, pDx2, pAlp1, pDy2, pDx3, pDx4, '
                'pAlp2) # default units: mm.')
        elif len(parameters) == 11:
            volume_definitions.append(
                f'gvolume.make_general_trapezoid({parameters[0]}, {parameters[1]}, {parameters[2]}, {parameters[3]}, '
                f'{parameters[4]}, {parameters[5]}, {parameters[6]}, {parameters[7]}, {parameters[8]}, '
                f'{parameters[9]}, {parameters[10]}) # default units: mm.')
        elif len(parameters) == 12:
            unit = check_units(parameters[11])
            volume_definitions.append(
                f'gvolume.make_general_trapezoid({parameters[0]}, {parameters[1]}, {parameters[2]}, {parameters[3]}, '
                f'{parameters[4]}, {parameters[5]}, {parameters[6]}, {parameters[7]}, {parameters[8]}, '
                f'{parameters[9]}, {parameters[10]}, \'{unit}\')')

    # elif volume_type == 'G4Trap8':

    else:
        print(f'\n Fatal error: {volume_type} not supported yet')
        exit(1)

    volume_definitions.append('gvolume.material = \'G4_AIR\'')
    if not silent:
        volume_definitions.append('# Uncomment any of the lines below to set parameters different than these defaults:\n')
        volume_definitions.append('# gvolume.mother = \'motherVolumeName\'                 # default: \'root\' ')
        volume_definitions.append('# gvolume.description = \'describe your volume here\'   # default: \'na\'')
        volume_definitions.append('# gvolume.set_position(myX, myY, myZ)                 # default: (0, 0, 0)')
        volume_definitions.append('# gvolume.set_rotation(myX, myY, myZ)                 # default: (0, 0, 0)')
        volume_definitions.append('# gvolume.mfield = \'solenoid\'                         # default: \'na\'')
        volume_definitions.append('# gvolume.color = \'838EDE\'                            # default: \'778899\'')
        volume_definitions.append('# gvolume.style = \'0\'                                 # (1 = surface, 0 = wireframe) default is 1')
        volume_definitions.append('# gvolume.visible = \'0\'                               # (1 = visible, 0 = invisible) default is 1')
        volume_definitions.append('# gvolume.digitization = \'flux\'                       # default: \'na\'')
        volume_definitions.append('# gvolume.set_identifier(\'paddleid\', 1)               # default: \'na\'')

    volume_definitions.append('gvolume.publish(configuration)')

    log_gvolume_from_defs(volume_definitions)


def log_gvolume_from_defs(volume_definitions):
    print()
    for vd in volume_definitions:
        print(f'	{vd}')
    print()


def print_all_g4solids():
    print(
        '\n The Geant4\033[91m solid constructors\033[0m are described at:\n\n '
        'https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomSolids'
        '.html\n')
    print(' The corresponding solids and their \033[92mconstructors\033[0m in gemc are:\n')
    for g4solid, description in AVAILABLE_SOLIDS_MAP.items():
        print(f'  - \033[91m{g4solid:20}\033[0m {description[0]:30}\n'
              f'    \033[92m{description[1]} \033[0m\n')
    print('\n\n')

import sys
if __name__ == "__main__":
    main()
