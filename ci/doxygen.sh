#!/usr/bin/env zsh

# Purpose: creates the doxygen documentation for the selected classes

# Container run:
# docker_run_image jeffersonlab/base:fedora36
#
# local build:
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/doxygen.sh goptions

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
