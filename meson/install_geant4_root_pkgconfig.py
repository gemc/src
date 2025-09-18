#!/usr/bin/env python3
"""
Generate pkg-config (.pc) files for Geant4 and (optionally) ROOT.

Usage:
    ./generate_pc.py <install-prefix>

Example:
    ./generate_pc.py /opt/gemc
"""
import subprocess
import shutil
import sys
import platform
from pathlib import Path
from typing import List, Optional

print(f"[debug] Python version: {sys.version}")
print(f"[debug] Python executable: {sys.executable}")
print(f"[debug] Detected machine: {platform.machine()}")

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


def is_arm64_host() -> bool:
	"""Return True on arm64/aarch64 hosts."""
	m = platform.machine().lower()
	return m in ("aarch64", "arm64")


def strip_arch_unsupported_flags(flags: str) -> str:
	"""
	On arm64 hosts, remove x86-specific -m* width flags that break compiles.
	(Some ROOT builds on Debian/Ubuntu inject '-m64' into --cflags.)
	"""
	toks = flags.split()
	if is_arm64_host():
		toks = [t for t in toks if t not in ("-m64", "-m32")]
	return " ".join(toks)


def filter_unwanted_flags(flags: str) -> str:
	"""Strip out Qt / CLHEP / XercesC / TreePlayer / -Wshadow flags."""
	unwanted = {"Qt", "qt", "CLHEP", "clhep", "xercesc", "XercesC", "TreePlayer", "-Wshadow"}
	return " ".join(f for f in flags.split() if not any(uw in f for uw in unwanted))


def filter_root_flags(flags: str, root_libs: List[str]) -> str:
	"""Keep only -l<lib> entries matching *root_libs* plus any -L paths."""
	return " ".join(
		f for f in flags.split()
		if f.startswith("-L") or any(f"-l{lib}" in f for lib in root_libs)
	)


def generate_pkgconfig(install_prefix: Path,
					   config_cmd: str,
					   output_filename: str,
					   name: str,
					   description: str,
					   root_lbs: Optional[List[str]] = None) -> None:
	"""Create <install_prefix>/lib/pkgconfig/<output_filename>."""
	prefix  = run_config(config_cmd, "--prefix")
	libs    = filter_unwanted_flags(run_config(config_cmd, "--libs"))
	cflags  = filter_unwanted_flags(run_config(config_cmd, "--cflags"))
	version = run_config(config_cmd, "--version")

	# Remove x86-only width flags on arm64 hosts (applies to both cflags and libs, just in case)
	cflags = strip_arch_unsupported_flags(cflags)
	libs   = strip_arch_unsupported_flags(libs)

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
		sys.exit(f"Usage: {sys.argv[0]} <install-prefix>")

	install_dir = Path(sys.argv[1]).expanduser().resolve()
	if not install_dir.exists():
		print(f"Creating installation directory {install_dir}")
		install_dir.mkdir(parents=True, exist_ok=True)

	# --- 2. generate .pc files -----------------------------------------------
	# generate_pkgconfig(install_dir, "geant4-config",
	#                    "geant4.pc", "Geant4", "Geant4 Simulation Toolkit")

	if check_root_config():
		generate_pkgconfig(
			install_dir,
			"root-config",
			"root.pc",
			"ROOT",
			"ROOT Data Analysis Framework",
			root_lbs=["RIO", "Tree", "Core", "root"],
		)
