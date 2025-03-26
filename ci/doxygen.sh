#!/usr/bin/env zsh

# Purpose: creates the doxygen documentation for the selected classes

# Container run:
# docker_run_image jeffersonlab/base:fedora36
#
# local build:
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/doxygen.sh

echo " "
echo " Doxygen version: "$(doxygen --version)
echo " "

classes=( goptions guts gfields glogging gfactory gtouchable ghit gtranslationTable gdata )

classes_to_do=(gdetector   gtranslationTable g4display gdynamicDigitization
 gsplash gui textProgressBar g4system gparticle gstreamer  userActions
gQtButtonsWidget  gphysics gsystem utilities eventDispenser   gsd  )

rm -rf pages ; mkdir pages
foreach class in $=classes; do
    echo " Running Doxygen for $class"
    cp doc/mydoxygen.css .
    cd $class
    ../ci/create_doxygen.sh $class # dummy copy to silent warning, not needed
    doxygen Doxyfile
    mkdir -p ../pages/$class
    mv html/* ../pages/$class
    cd ..
    echo
done
./ci/generate_html.py
