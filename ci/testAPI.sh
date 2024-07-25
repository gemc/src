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

function tests_templates {
  echo " > Testing helper"
  $GEMC/api/templates.py
  echo " > Testing solid list helper"
  $GEMC/api/templates.py -sl
  for solid in G4Box G4Tubs G4Cons G4Trd G4TrapRAW G4TrapG; do
    echo " > Testing solid code for $solid"
    $GEMC/api/templates.py -gv $solid
  done
}

set_ld_path
echo
tests_templates
