#!/usr/bin/env zsh

# Purpose: Runs the examples in clas12-system

# Container run example:
# docker run -it --rm jeffersonlab/gemc:3.0 bash
# git clone http://github.com/gemc/glibrary /root/glibrary && cd /root/glibrary
# ./ci/runExamples.sh -e dosimeter

# load environment if we're on the container
# notice the extra argument to the source command
TERM=xterm # source script use tput for colors, TERM needs to be specified
FILE=/etc/profile.d/jlab.sh
test -f $FILE && source $FILE keepmine

Help()
{
	# Display Help
	echo
	echo "Syntax: runExamples.sh [-h|e]"
	echo
	echo "Options:"
	echo
	echo "-h: Print this Help."
	echo "-e <example>: runs sci-g examples"
	echo
}

if [ $# -eq 0 ]; then
	Help
	exit 1
fi

while getopts ":he:" option; do
   case $option in
      h)
         Help
         exit
         ;;
      e)
         detector=$OPTARG
         ;;
     \?) # Invalid option
         echo "Error: Invalid option"
         exit 1
         ;;
   esac
done

./ci/build.sh # build gemc

cd $JLAB_SOFTWARE/clas12-systems/$G3CLAS12_VERSION
./ci/tests.sh -s $detector -t
