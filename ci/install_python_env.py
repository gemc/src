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

prefix    = Path(os.environ['MESON_INSTALL_PREFIX'])
pygemc_src = sys.argv[1]
venv_dir  = prefix / 'python_env'

print(f' > Installing Python environment at {venv_dir}')
subprocess.run([sys.executable, '-m', 'venv', str(venv_dir)], check=True)

venv_python = str(venv_dir / 'bin' / 'python3')
subprocess.run([venv_python, '-m', 'pip', 'install', '--no-cache-dir',
                '--upgrade', 'pip', 'setuptools'], check=True)
# --no-build-isolation avoids pip spawning an extra isolated build env, reducing memory pressure.
subprocess.run([venv_python, '-m', 'pip', 'install', '--no-cache-dir',
                '--no-build-isolation', pygemc_src], check=True)
