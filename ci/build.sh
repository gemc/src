#!/usr/bin/env zsh

# Purpose: compiles gemc and installs it in gemc

# Container run:
# docker run -it --rm jeffersonlab/gemc3:g3vdev-g4v11.2.1-almalinux93-local
# git clone http://github.com/gemc/src /root/src && cd /root/src
# git clone http://github.com/maureeungaro/src /root/src && cd /root/src
# ./ci/build.sh

# if we are in the docker container, we need to load the modules
if [[ -z "${DISTTAG}" ]]; then
    echo "\nNot in container"
else
    echo "\nIn container: ${DISTTAG}"
    source  /app/localSetup.sh
fi

function compileGEMC {
	meson setup build --native-file=release.ini -Duse_root=true --wipe
	cd build
	meson configure -Dprefix=$GEMC
	meson install
	meson test
	cd ..
}

compileGEMC
echo
echo "- Content of $GEMC"
ls -lrt $GEMC
echo "- Content of $GEMC/bin"
ls -lrt $GEMC/bin
echo "- Content of $GEMC/lib"
ls -lrt $GEMC/lib
