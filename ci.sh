#!/usr/bin/env bash
set -e

# GEMC Continuous Integration
# ----------------------------
#
# To debug this on the container:
#
# docker run -it --rm jeffersonlab/gemc:3.0 bash
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci.sh

# load environment if we're on the container
FILE=/etc/profile.d/jlab.sh
if test -f "$FILE"; then
    source "$FILE"
fi

function compileGEMC {
	# getting number of available CPUS
	copt=" -j"`getconf _NPROCESSORS_ONLN`" OPT=1"
	echo
	echo Compiling GEMC with options: "$copt"
	scons $copt
	# checking existance of executable
	ls gemc
	if [ $? -ne 0 ]; then
		echo gemc executable not found
		exit 1
	fi
}

# this uses the $GEMC where sci-g is expected and $GPLUGIN_PATH
function runScigCIExamples {
	# each job need to compile gemc
	# this is not a problem, compilation is fast
	compileGEMC

	# currently we need to run the script to produce the geometry
	# we can change this to use the MYSQL database when it is ready
	cd $GEMC/sci-g

	./ci.sh "$1" "$2" "$3"
}

function runScigCITargets {
	# each job need to compile gemc
	# this is not a problem, compilation is fast
	compileGEMC

	# currently we need to run the script to produce the geometry
	# we can change this to use the MYSQL database when it is ready
	cd $GEMC/sci-g

	./ci.sh "$1"
}

echo
echo "GEMC Validation:"
echo
time=$(date)
echo "::set-output name=time::$time"

if [ $# -eq 3 ]; then
	echo "Running gemc compilation and sci-gi check:" "$1" "$2" "$3"
	runScigCIExamples "$1" "$2" "$3"
elif [ $# -eq 1 ]; then
	runScigCITargets "$1"
fi


