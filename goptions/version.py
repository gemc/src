#!/usr/bin/python3

import subprocess
from datetime import datetime

def get_git_version():
    try:
        # Run the git command to get the latest tag
        tag = subprocess.check_output(['git', 'describe', '--tags', '--abbrev=0']).strip().decode('utf-8')
        # Run another git command to get the date of the tag
        date = subprocess.check_output(['git', 'log', '-1', '--format=%ai', tag]).strip().decode('utf-8')
        return tag, datetime.strptime(date, '%Y-%m-%d %H:%M:%S %z').strftime('%Y-%m-%d')
    except subprocess.CalledProcessError:
        print("Error: Unable to get git version.")
        return None, None


def generate_header_file(version, release_date):
    header_content = f"""\
#ifndef VERSION_H
#define VERSION_H

const char* gversion = "{version}";
const char* grelease_date = "{release_date}";
const char* greference = "Nucl. Instrum. Meth. A, Volume 959, 163422 (2020)";
const char* gweb = "https://gemc.github.io/home/";
const char* gauthor = "Maurizio Ungaro, ungaro@jlab.org";

#endif // VERSION_H
"""
    with open('gversion.h', 'w') as header_file:
        header_file.write(header_content)


def main():
    version, release_date = get_git_version()
    if version and release_date:
        generate_header_file(version, release_date)
        print("Version header file 'gversion.h' generated successfully.")
    else:
        print("Failed to generate version header file.")


if __name__ == "__main__":
    main()
