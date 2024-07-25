#!/usr/bin/env python3

# imports: do not edit these lines
import argparse
import logging
import os

from gemc_api_geometry import *

_logger = logging.getLogger("sci-g")

NGIVEN: str = 'NOTGIVEN'
NGIVENS: [str] = ['NOTGIVEN']

from solids_map import AVAILABLE_SOLIDS_MAP, g4htmlImages


def main():
    print_html_g4solids()


def print_html_g4solids():
    doc_string: str = '---\n' \
                      'layout: default\n' \
                      'title: Build Volumes from Solid Types\n' \
                      'description: use python to create volumes based on geant4 solids\n' \
                      '---\n' \
                      'This document describes how to use python to build the volumes described in the ' \
                      '<a href="https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/' \
                      'Detector/Geometry/geomSolids.html">Geant4 User Guide</a><br/><br/>\n' \
                      'The volumes are built within a system using the python interface. <br/>\n' \
                      '<br/><br/>Each geant4 solid\'s constructor is documented below.<br/><br/>\n'

    """html table with 5 columns filled with all AVAILABLE_SOLIDS_MAP keys """
    doc_string += '<table style="width:60% ">\n'
    doc_string += '<tr>\n'
    empty_var = ' '
    for i, g4solid in enumerate(AVAILABLE_SOLIDS_MAP.keys()):
        image_link = f'{g4htmlImages}{AVAILABLE_SOLIDS_MAP[g4solid][2]}'
        if i % 4 == 0:
            doc_string += '</tr>\n'
            doc_string += '<tr>\n'
        doc_string += f'    <td><a href="#{g4solid}">{g4solid}</a>{empty_var:20}<img src="{image_link}" style="width: ' \
                      f'30px; height: 30px; padding: 0px"/></td>\n'
    doc_string += '</tr>\n'
    doc_string += '</table><br/><br/>\n'

    for g4solid, description in AVAILABLE_SOLIDS_MAP.items():
        doc_string += f'<h4 id="{g4solid}">{g4solid}: <i>{description[0]}</i> </h4>\n'
        doc_string += '<div class="align-items-center">\n'
        doc_string += '\t<p>\n'

        solid_method = getattr(GVolume, description[1])
        function_docs_lines = str(solid_method.__doc__).splitlines()
        for d_line in function_docs_lines:
            stripped_line = d_line.strip()
            if 'Parameters' in d_line:
                doc_string += f'\t\t<i><b>{stripped_line}:</b></i> <br/>\n'
            elif '----' in d_line:
                doc_string += '\n'
            elif 'Example' in d_line:
                doc_string += f'\t\t<br/>\n\t\t<i><b>{stripped_line}:</b></i> <br/>\n'
            elif '>' in d_line:
                doc_string += f'\t\t<p style="font-family:courier;">{stripped_line}</p>\n'
            elif description[1] in d_line:
                doc_string += f'\t\tFunction: <b>{stripped_line}</b><br/>\n'
            elif stripped_line == '':
                doc_string += '\t\t<br/>\n'
            else:
                doc_string += f'\t\t{stripped_line}<br/>\n'

        doc_string += '\t</p>\n\t\t\n'
        doc_string += '\t<div>\n\t\t<img  '
        doc_string += f'src="{g4htmlImages}{description[2]}"/>\n\t</div>\n'

        doc_string += '</div>\n<hr style="color:black; opacity: 0.8"><br/>\n\n'

    jekyll_file_name = '/opt/projects/gemc/home/_documentation/geometryDocs/solidTypes.md'
    with open(jekyll_file_name, 'w') as dn:
        dn.write(doc_string)


if __name__ == "__main__":
    main()
