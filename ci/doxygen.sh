#!/usr/bin/env zsh

# Purpose: creates the doxygen documentation for the selected classes

# Container run:
# docker_run_image jeffersonlab/geant4:g4v11.2.2-almalinux93
# docker_run_image jeffersonlab/geant4:g4v11.2.2-ubuntu24
# docker_run_image jeffersonlab/geant4:g4v11.2.2-fedora36
#
# local build:
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/doxygen.sh goption

echo " "
echo " Doxygen version: "$(doxygen --version)
echo " "

class=$1
cd $class

echo
../ci/create_doxygen.sh
cp ../doc/doxygen.css .
echo " Running Doxygen for "$class
doxygen Doxyfile
echo
echo done
