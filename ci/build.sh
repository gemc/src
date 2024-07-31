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

echo " > Running build Configure"
meson setup build --native-file=release.ini -Duse_root=true --wipe
cd build
meson configure -Dprefix=$GEMC
echo " > Running meson compile and install"
meson compile -v
meson install
show_installation
