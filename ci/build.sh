#!/usr/bin/env zsh

# Purpose: compiles gemc and installs it in gemc

# Container run:
# docker run -it --rm jeffersonlab/gemc:3.0 sh
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh

# load environment if we're on the container
# notice the extra argument to the source command
TERM=xterm # source script use tput for colors, TERM needs to be specified
FILE=/etc/profile.d/jlab.sh
test -f $FILE && source $FILE keepmine

function compileGEMC {
	# getting number of available CPUS
	copt=" -j"`getconf _NPROCESSORS_ONLN`" OPT=1"
	echo
	echo Compiling GEMC with options: "$copt"
	scons SHOWENV=1 SHOWBUILD=1 $copt
	# checking existance of executable
	ls gemc
	if [ $? -ne 0 ]; then
		echo gemc executable not found
		exit 1
	fi
}

compileGEMC
cp gemc $GEMC
ls -lrt $GEMC
which gemc
