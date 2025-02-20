#!/usr/bin/env zsh

# Purpose: compiles, installs gemc, run tests

# Container run:
# docker_run_image jeffersonlab/geant4:g4v11.2.2-almalinux93
# docker_run_image jeffersonlab/geant4:g4v11.2.2-ubuntu24
# docker_run_image jeffersonlab/geant4:g4v11.2.2-fedora36
#
# local build:
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh leak


source ci/env.sh
sanitize_option="-Db_sanitize=$1"
pgo=""
buildtype="-Dbuildtype=debug"

# if on ubuntu, use pgo generate
if [ -f /etc/os-release ]; then
    if grep -q "Ubuntu" /etc/os-release; then
        pgo="-Db_pgo=generate"
    fi
fi

echo " > Running build Configure"
meson setup build --native-file=core.ini -Duse_root=true $sanitize_option $buildtype $pgo -Dprefix=$GEMC --wipe
cd build
echo " > Running meson compile and install"
meson compile -v
meson install
meson test -v
show_installation
