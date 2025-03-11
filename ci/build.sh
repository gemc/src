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
echo " > Current directory: $(pwd) content:"
ls -l || exit 1
meson install  || exit 1
echo " > $GEMC recursive content:"
ls -l $GEMC || exit 1

# if $1 is NOT one of sanitize option, run meson test
if [[ $1 != @(address|thread|undefined|memory|leak) ]]; then
    echo " > Running meson test"
    meson test -v || exit 1
fi

