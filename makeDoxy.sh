#!/bin/zsh

# Purpose: creates the doxygen documentation for all the glibraries
# This script is used by TRAVIS
#
# Arguments: none

echo " "
echo " Doxygen version: "$(doxygen --version)
echo " "

# TRAVISENVIRONMENT is defined in .travis.yml
if [[ -v TRAVISENVIRONMENT ]]; then
	echo " Travis Build" >&2
	git clone https://github.com/gemc/glibrary.git

	./glibrary/createDoxyfile.sh
	./glibrary/makeDoxyfileForLibrary.sh gemc

else
	echo " Non travis Build" >&2
	../glibrary/createDoxyfile.sh
	../glibrary/makeDoxyfileForLibrary.sh gemc
fi


echo " "Creating gemc doxygen documentation
doxygen Doxyfile
echo " "
