#!/usr/bin/env python3

import subprocess
import os


def run_config(command, option):
    """Run a config command with the given option and return the output."""
    result = subprocess.run([command, option], capture_output=True, text=True, check=True)
    return result.stdout.strip()


def filter_unwanted_flags(flags):
    """Remove unwanted flags related to Qt, CLHEP, and XercesC."""
    #unwanted = ["CLHEP", "clhep", "xercesc", "XercesC", "TreePlayer", "-Wshadow"]
    unwanted = ["Qt", "qt", "CLHEP", "clhep", "xercesc", "XercesC", "TreePlayer", "-Wshadow"]
    return " ".join(flag for flag in flags.split() if not any(uw in flag for uw in unwanted))


def filter_root_flags(flags, root_list):
    """Keep only specified ROOT libraries."""
    return " ".join(flag for flag in flags.split() if "-L" in flag or any(f"-l{lib}" in flag for lib in root_list))


def generate_pkgconfig(config_cmd, output_filename, name, description, root_list=None):
    """Generate a .pc file for pkg-config."""
    prefix = run_config(config_cmd, "--prefix")
    libs = filter_unwanted_flags(run_config(config_cmd, "--libs"))
    cflags = filter_unwanted_flags(run_config(config_cmd, "--cflags"))
    version = run_config(config_cmd, "--version")

    if config_cmd == "root-config" and root_list:
        libs = filter_root_flags(libs, root_list)
    else:
        libs = filter_unwanted_flags(libs)
        cflags = filter_unwanted_flags(cflags)

    pc_content = f"""
prefix={prefix}
exec_prefix=${{prefix}}
libdir=${{prefix}}/lib
includedir=${{prefix}}/include
   
Name: {name}
Description: {description}
Version: {version}
Cflags: {cflags}
Libs:  {libs}
    """.strip()

    # Write to the .pc file
    pc_file_path = os.path.join(os.environ["GEMC"], "lib", "pkgconfig", output_filename)
    os.makedirs(os.path.dirname(pc_file_path), exist_ok=True)
    with open(pc_file_path, "w") as pc_file:
        pc_file.write(pc_content + "\n")

    print(f"Generated {pc_file_path}")


if __name__ == "__main__":
    generate_pkgconfig("geant4-config", "geant4.pc", "Geant4", "Geant4 Simulation Toolkit")
    root_list = ["RIO", "Tree", "Core", "root"]
    generate_pkgconfig("root-config", "root.pc", "ROOT", "ROOT Data Analysis Framework", root_list)
