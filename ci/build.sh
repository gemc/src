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
# if we are on an ubuntu OS, need the profile ocmpiler options to complete, no idea why
if [[ $(cat /etc/os-release | grep -i ubuntu) ]]; then
  echo " > Running meson configure -Dprefix=$GEMC  -Db_pgo=\'generate\'  "
  meson configure -Dprefix=$GEMC -Db_pgo='generate'
else
  echo " > Running meson configure -Dprefix=$GEMC"
  meson configure -Dprefix=$GEMC
fi
echo " > Running meson compile and install"
meson compile -v
meson install
show_installation
