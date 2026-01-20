#!/usr/bin/env python3
"""
Generate pkg-config (.pc) file for Geant4

Writes:
  - geant4.pc        : full libs from geant4-config --libs (as before)
  - geant4_core.pc   : libs with graphical/GUI/vis/X11/OpenGL/Qt removed

Usage:
    ./g4_pkgconfig.py <install-prefix>

Example:
    ./g4_pkgconfig.py $GEMC
"""
import subprocess
import sys
from pathlib import Path
from typing import List, Optional

print(f"[debug] Python version: {sys.version}")
print(f"[debug] Python executable: {sys.executable}")


# ────────────────────────── helpers ──────────────────────────
def run_config(command: str, option: str) -> str:
	try:
		result = subprocess.run([command, option], capture_output=True, text=True, check=True)
		return result.stdout.strip()
	except FileNotFoundError:
		print(f"[FATAL] Command not found: {command}")
		raise
	except subprocess.CalledProcessError as e:
		print(f"[ERROR] Command failed: {command} {option}")
		print(f"stdout:\n{e.stdout}")
		print(f"stderr:\n{e.stderr}")
		raise


def filter_unwanted_flags(flags: str) -> str:
	"""Strip out Qt / CLHEP / XercesC / TreePlayer / -Wshadow flags."""
	unwanted = {
		"Qt", "qt",
		"CLHEP", "clhep",
		"xercesc", "XercesC",
		"TreePlayer",
		"-Wshadow",
	}
	return " ".join(f for f in flags.split() if not any(uw in f for uw in unwanted))


def filter_graphical_libs(flags: str) -> str:
	"""
    Remove graphical/GUI/visualization-related link flags from a libs string.

    This aims to eliminate X11 / Xt / Xmu / Xext / Xcb / OpenGL / GLU / GLUT / Qt* /
    and common Geant4 visualization libraries (G4OpenGL*, G4vis_* etc.) if they
    appear explicitly.

    Note: This filter is intentionally conservative; it removes tokens whose text
    clearly indicates GUI/vis, and also removes absolute paths to corresponding
    static/shared libs.
    """
	# Substrings that identify GUI/vis tokens (works for -lX11, /usr/lib/.../libX11.a, etc.)
	gui_markers = (
		# X11 family
		"X11", "Xau", "Xdmcp", "Xext", "Xmu", "Xt", "xcb", "ICE", "SM",
		# OpenGL family
		"GLU", "glut", "OpenGL", "libGL", "-lGL", "GLX",
		# Qt
		"Qt5", "Qt6", "Qt",
		# Geant4 visualization libs (common names)
		"G4OpenGL", "G4RayTracer", "G4VRML", "G4GMocren", "G4vis",
		"G4ToolsSG",
	)

	kept: List[str] = []
	for tok in flags.split():
		# Drop anything that obviously references GUI/vis
		if any(m in tok for m in gui_markers):
			continue

		# Also drop common linker options that can appear adjacent to GUI/vis libs,
		# but only when they are standalone tokens; keep everything else.
		# (We intentionally do NOT remove -Wl,*, --start-group, etc.)
		kept.append(tok)

	return " ".join(kept)


def generate_pkgconfig(
		install_prefix: Path,
		config_cmd: str,
		output_filename: str,
		name: str,
		description: str,
		libs_filter=None,
) -> None:
	"""Create <install_prefix>/lib/pkgconfig/<output_filename>."""
	prefix = run_config(config_cmd, "--prefix")

	libs_raw = run_config(config_cmd, "--libs")
	cflags_raw = run_config(config_cmd, "--cflags")
	version = run_config(config_cmd, "--version")

	# Keep your existing filtering for cflags; libs can be optionally post-filtered.
	cflags = filter_unwanted_flags(cflags_raw)
	libs = filter_unwanted_flags(libs_raw)

	if libs_filter is not None:
		libs = libs_filter(libs)

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


# ────────────────────────── main ──────────────────────────
if __name__ == "__main__":
	# --- 1. parse argument ----------------------------------------------------
	if len(sys.argv) != 2:
		sys.exit(f"Usage: {sys.argv[0]} <install-prefix>")

	install_dir = Path(sys.argv[1]).expanduser().resolve()
	if not install_dir.exists():
		print(f"Creating installation directory {install_dir}")
		install_dir.mkdir(parents=True, exist_ok=True)

	# --- 2. generate .pc files -----------------------------------------------
	# Full (as before)
	generate_pkgconfig(
		install_dir,
		"geant4-config",
		"geant4.pc",
		"Geant4",
		"Geant4 Simulation Toolkit",
	)

	# Core (no graphical libs)
	generate_pkgconfig(
		install_dir,
		"geant4-config",
		"geant4_core.pc",
		"Geant4 Core",
		"Geant4 Simulation Toolkit (core, no graphical/GUI libs)",
		libs_filter=filter_graphical_libs,
	)
