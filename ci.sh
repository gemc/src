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
function runScigCI {
	local dir="$1"
	local script="$2"
	local gcard="$3"
	local scig=$GEMC/sci-g

	echo using plugins at $GPLUGIN_PATH
	echo using sci-g at $scig

	# each job need to compile gemc
	# this is not a problem, compilation is fast
	compileGEMC

	# currently we need to run the script to produce the geometry
	# we can change this to use the MYSQL database when it is ready
	cd $scig

	./ci.sh "$1" "$2" "$3"
}

echo
echo "GEMC Validation:"
echo
time=$(date)
echo "::set-output name=time::$time"

if [ $# -eq 3 ]; then
	echo "Running individual check:" "$1" "$2" "$3"
	runJcards "$1" "$2" "$3"
else
	echo "Running all checks"
	runAll
fi

function runAll {
	runScigCI examples/geometry/simple_flux example.py example.json
	runScigCI examples/geometry/dosimeter example.py example.json
	runScigCI projects/clas12/targets targets.py target_lh2.jcard
	runScigCI projects/clas12/targets targets.py target_c12.jcard

	#runScigCI examples/plugins/calorimeter  calorimeter.py example.jcard
}
