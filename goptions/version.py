#!/usr/bin/python3
"""
! @file version.py
! @brief Generates the C++ header `gversion.h` from Git metadata.
!
! @details
! This script is typically run as part of a build or release pipeline.
! It queries Git for:
! - the most recent tag (or commit identifier as fallback),
! - the commit date associated with that tag,
! and writes those into a generated header file `gversion.h`.
!
! The resulting header provides global C-string constants used by \ref GOptions : to
! print and persist version information.
"""

import subprocess
from datetime import datetime

def get_git_version():
    """
    ! @brief Retrieves version and release date from Git.
    !
    ! @details
    ! - Version is the latest annotated tag (or a commit identifier if no tags exist),
    !   as returned by: `git describe --tags --abbrev=0 --always`.
    ! - Release date is obtained by looking up the commit date for that tag via:
    !   `git log -1 --format=%ai <tag>`.
    !
    ! @return A tuple `(version_tag, release_date)` where `release_date` is formatted as `YYYY-MM-DD`.
    !         Returns `(None, None)` on failure.
    """
    try:
        # Run the git command to get the latest tag
        tag = subprocess.check_output(['git', 'describe', '--tags', '--abbrev=0', '--always']).strip().decode('utf-8')
        # Run another git command to get the date of the tag
        date = subprocess.check_output(['git', 'log', '-1', '--format=%ai', tag]).strip().decode('utf-8')
        return tag, datetime.strptime(date, '%Y-%m-%d %H:%M:%S %z').strftime('%Y-%m-%d')
    except subprocess.CalledProcessError:
        print("Error: Unable to get git version.")
        return None, None


def generate_header_file(version, release_date):
    """
    ! @brief Writes the generated C++ header `gversion.h`.
    !
    ! @details
    ! The header defines global C-string constants:
    ! - `gversion`
    ! - `grelease_date`
    ! - `greference`
    ! - `gweb`
    ! - `gauthor`
    !
    ! These are included by C++ code (e.g., `goptions.cc`) to display version information and
    ! to save it into YAML configuration output.
    !
    ! @param version Version string derived from Git.
    ! @param release_date Release date string formatted as `YYYY-MM-DD`.
    """
    header_content = f"""\
#pragma once

const char* gversion = "{version}";
const char* grelease_date = "{release_date}";
const char* greference = "Nucl. Instrum. Meth. A, Volume 959, 163422 (2020)";
const char* gweb = "https://gemc.github.io/home";
const char* gauthor = "Maurizio Ungaro, ungaro@jlab.org";

"""
    with open('gversion.h', 'w') as header_file:
        header_file.write(header_content)


def main():
    """
    ! @brief Script entry point: queries Git and generates `gversion.h`.
    !
    ! @details
    ! If Git metadata cannot be obtained, the script prints an error message and does not
    ! generate the header file.
    """
    version, release_date = get_git_version()
    if version and release_date:
        generate_header_file(version, release_date)
        print("Version header file 'gversion.h' generated successfully.")
    else:
        print("Failed to generate version header file.")


if __name__ == "__main__":
    main()
