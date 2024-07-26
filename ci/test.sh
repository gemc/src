#!/usr/bin/env zsh

# Purpose: compiles, installs gemc, run tests

# Container run:
# docker_run_image jeffersonlab/geant4:g4v11.2.2-almalinux93
# docker_run_image jeffersonlab/geant4:g4v11.2.2-ubuntu24
# docker_run_image jeffersonlab/geant4:g4v11.2.2-fedora36
#
# local build:
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh

source ci/functions.sh

set_ld_path
cd build
ls -l
echo " > Running meson tests"
meson test -v
$GEMC/bin/gemc -v
if [ $? -ne 0 ]; then
  echo Running gemc  failed
  exit 1
fi
