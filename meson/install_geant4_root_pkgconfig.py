#!/usr/bin/env python3
"""
Generate pkg‑config (.pc) files for Geant4 and (optionally) ROOT.

Usage:
    ./generate_pc.py <install‑prefix>

Example:
    ./generate_pc.py /opt/gemc
"""
import subprocess
import shutil
import sys
from pathlib import Path


# ────────────────────────── helpers ──────────────────────────
def run_config(command: str, option: str) -> str:
	"""Run <command> <option> and return its stdout."""
	result = subprocess.run([command, option],
							capture_output=True, text=True, check=True)
	return result.stdout.strip()


def filter_unwanted_flags(flags: str) -> str:
	"""Strip out Qt / CLHEP / XercesC / TreePlayer / -Wshadow flags."""
	unwanted = {"Qt", "qt", "CLHEP", "clhep",
				"xercesc", "XercesC", "TreePlayer", "-Wshadow"}
	return " ".join(f for f in flags.split()
					if not any(uw in f for uw in unwanted))


def filter_root_flags(flags: str, root_libs: list[str]) -> str:
	"""Keep only -l<lib> entries matching *root_libs* plus any -L paths."""
	return " ".join(f for f in flags.split()
					if f.startswith("-L") or
					any(f"-l{lib}" in f for lib in root_libs))


def generate_pkgconfig(install_prefix: Path,
					   config_cmd: str,
					   output_filename: str,
					   name: str,
					   description: str,
					   root_lbs: list[str] | None = None) -> None:
	"""Create <install_prefix>/lib/pkgconfig/<output_filename>."""
	prefix   = run_config(config_cmd, "--prefix")
	libs     = filter_unwanted_flags(run_config(config_cmd, "--libs"))
	cflags   = filter_unwanted_flags(run_config(config_cmd, "--cflags"))
	version  = run_config(config_cmd, "--version")

	if config_cmd == "root-config" and root_lbs:
		libs = filter_root_flags(libs, root_lbs)

	pc_content = f"""\
prefix={prefix}
exec_prefix=${{prefix}}
libdir=${{prefix}}/lib
includedir=${{prefix}}/include

Name: {name}
Description: {description}
Version: {version}
Cflags: {cflags}
Libs:  {libs}
"""

	pc_path = install_prefix / "lib" / "pkgconfig" / output_filename
	pc_path.parent.mkdir(parents=True, exist_ok=True)
	pc_path.write_text(pc_content + "\n")
	print(f"Generated {pc_path}")


def check_root_config() -> bool:
	return shutil.which("root-config") is not None


# ────────────────────────── main ──────────────────────────
if __name__ == "__main__":
	# --- 1. parse argument ----------------------------------------------------
	if len(sys.argv) != 2:
		sys.exit(f"Usage: {sys.argv[0]} <install‑prefix>")

	install_dir = Path(sys.argv[1]).expanduser().resolve()
	if not install_dir.exists():
		print(f"Creating installation directory {install_dir}")
		install_dir.mkdir(parents=True, exist_ok=True)

	# --- 2. generate .pc files -----------------------------------------------
	generate_pkgconfig(install_dir, "geant4-config",
					   "geant4.pc", "Geant4", "Geant4 Simulation Toolkit")

	if check_root_config():
		generate_pkgconfig(install_dir, "root-config",
						   "root.pc", "ROOT", "ROOT Data Analysis Framework",
						   root_lbs=["RIO", "Tree", "Core", "root"])
