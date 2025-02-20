#!/usr/bin/env zsh

# Purpose: compiles gemc with sanitizers

# Container run:
# docker run -it --rm --platform linux/amd64 jeffersonlab/gemc:dev-fedora36 sh
# git clone http://github.com/gemc/clas12Tags /root/clas12Tags && cd /root/clas12Tags
# ./ci/build_gemc.sh

source ci/env.sh

sanitize_option=$(sanitize_options $1)

echo " > Running build Configure"
meson setup build --native-file=core.ini -Duse_root=true $sanitize_option -Dprefix=$GEMC --wipe
cd build
echo " > Running meson compile and install"
meson compile -v

# if