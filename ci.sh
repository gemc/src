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
function runJcards {
	local dir="$1"
	local gcard="$2"
	local scig=$GEMC

	echo using plugins at $GPLUGIN_PATH
	echo using sci-g at $scig

	# each job need to compile gemc
	# this is not a problem, compilation is fast
	compileGEMC

	local jcard=$scig/$dir/$gcard
	./gemc $jcard
}

echo
echo "GEMC Validation: $1"
echo
time=$(date)
echo "::set-output name=time::$time"

if [ $# -eq 2 ]; then
	echo "Running individual check:" "$1" "$2"
	runJcards "$1" "$2"
else
	echo "Running all checks"
	runAll
fi

function runAll {
	runJcards examples/geometry/simple_flux example.jcard
	runJcards examples/plugins/calorimeter  example.jcard
	runJcards projects/clas12/targets       target.jcard
}
