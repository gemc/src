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

source ci/functions.sh
sanitize_option="-Db_sanitize=$1"

# reset sanitize option if on mac
if [[ "$OSTYPE" == "darwin"* ]] || [[ 'standard' == $1 ]]; then
    sanitize_option=""
fi

echo " > Running build Configure"
meson setup build --native-file=release.ini -Duse_root=true $sanitize_option -Dprefix=$GEMC --wipe
cd build
echo " > Running meson compile and install"
meson compile -v
meson install
show_installation
