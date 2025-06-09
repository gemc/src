#!/usr/bin/env zsh

# Purpose: compiles gemc with optional sanitizers or debugging options.

# Container run:
# docker run -it --rm --platform linux/amd64 jeffersonlab/geant4:g4v11.3.1-almalinux94 sh
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh none

source ci/env.sh

meson_option=$(meson_options $1)
max_threads=$(max_j)

setup_options=" --native-file=core.ini -Duse_root=true $meson_option -Dprefix=$GEMC --wipe "

echo " > Running build Configure with setup build options: $setup_options"
meson setup build $=setup_options || exit 1

cd build  || exit 1

echo " > Running meson compile -v  -j $max_threads"
meson compile -v  -j $max_threads || exit 1

echo " > Current directory: $(pwd) content:"
ls -l || exit 1

echo " > Running meson install"
meson install  || exit 1

echo " > $GEMC recursive content:"
ls -lR $GEMC || exit 1

echo
echo " ldd of $GEMC/bin/gemc:"

# if on unix, use ldd , if on mac, use otool -L
if [[ "$(uname)" == "Darwin" ]]; then
  otool -L $GEMC/bin/gemc || exit 1
else
  ldd $GEMC/bin/gemc || exit 1
fi



echo

# if $1 is NOT one of sanitize option, run meson test
if [[ $1 != @(address|thread|undefined|memory|leak) ]]; then
    echo " > Running meson test"
    meson test -v -j 1 || exit 1
fi

