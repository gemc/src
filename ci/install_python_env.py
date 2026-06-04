#!/usr/bin/env python3
"""
Install the pygemc Python environment into the install prefix.

Called by meson.add_install_script() during 'meson install'.
Creates ${prefix}/python_env/ with pygemc and all its dependencies so that
users only need to add ${prefix}/python_env/bin to PATH.

Environment variables set by meson:
  MESON_INSTALL_PREFIX  -- effective install prefix (respects DESTDIR)

Arguments:
  sys.argv[1]  -- absolute path to the pygemc source directory
"""
import os
import subprocess
import sys
from pathlib import Path

if os.environ.get('GEMC_SKIP_PYTHON_ENV_INSTALL') == '1':
    print(' > Skipping Python environment installation (GEMC_SKIP_PYTHON_ENV_INSTALL=1)')
    sys.exit(0)

prefix     = Path(os.environ['MESON_INSTALL_PREFIX'])
pygemc_src = sys.argv[1]
venv_dir   = prefix / 'python_env'
venv_python = str(venv_dir / 'bin' / 'python3')

if venv_dir.exists():
    print(f' > Python environment already exists at {venv_dir}, updating pygemc only')
    subprocess.run([venv_python, '-m', 'pip', 'install', pygemc_src], check=True)
    sys.exit(0)

print(f' > Installing Python environment at {venv_dir}')
subprocess.run([sys.executable, '-m', 'venv', str(venv_dir)], check=True)
subprocess.run([venv_python, '-m', 'pip', 'install', '--upgrade',
                'pip', 'setuptools', 'wheel'], check=True)
subprocess.run([venv_python, '-m', 'pip', 'install', pygemc_src], check=True)
