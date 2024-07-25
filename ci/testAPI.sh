#!/usr/bin/env zsh

# Purpose: compiles, installs gemc, run tests

# Container run:
# docker_run_image jeffersonlab/geant4:g4v11.2.2-almalinux93
# docker_run_image jeffersonlab/geant4:g4v11.2.2-ubuntu24
# docker_run_image jeffersonlab/geant4:g4v11.2.2-fedora36
#
# local build:
# git clone http://github.com/gemc/src /root/src && cd /root/src
# git clone http://github.com/maureeungaro/src /root/src && cd /root/src
# ./ci/build.sh

source ci/functions.sh

function tests {
  cd build
  echo " > Running meson test"
	meson test -v
	cd ..
  $GEMC/bin/gemc -v
  if [ $? -ne 0 ]; then
    echo Running gemc  failed
    exit 1
  fi
}

set_ld_path
echo
tests
