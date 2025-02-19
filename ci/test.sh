#!/usr/bin/env zsh

# Purpose: compiles, installs gemc, run tests

# Container run:
# docker_run_image jeffersonlab/geant4:g4v11.3.0-almalinux93
#
# local build:
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh

source ci/functions.sh

cd build
ls -l
echo " > Running meson tests"
meson test -v
