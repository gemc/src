#!/usr/bin/env zsh

# Purpose: compiles gemc with sanitizers

# Container run:
# docker run -it --rm --platform linux/amd64 jeffersonlab/geant4:g4v11.3.0-almalinux94 sh
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh none

source ci/env.sh

sanitize_option=$(sanitize_options $1)

setup_options=" --native-file=core.ini -Duse_root=true $sanitize_option -Dprefix=$GEMC --wipe "
echo " > Running build Configure with setup build options: $setup_options"

meson setup build $=setup_options || exit 1
cd build  || exit 1
echo " > Running meson compile and install"
meson compile -v  || exit 1
ls -l build || exit 1
meson install  || exit 1
ls -l $GEMC/lib || exit 1
ls -R $GEMC/examples || exit 1
meson test -v || exit 1

