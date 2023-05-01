#!/usr/bin/env zsh

# Purpose: Runs the examples in sci-g

# Container run:
# docker run -it --rm jeffersonlab/gemc:3.0 sh
# git clone http://github.com/gemc/src /root/src && cd /root/src
# git clone http://github.com/maureeungaro/src /root/src && cd /root/src
# ./ci/testSciG.sh -e examples/geometry/dosimeter

# if we are in the docker container, we need to load the modules
if [[ -z "${DISTTAG}" ]]; then
    echo "\nNot in container"
else
    echo "\nIn container: ${DISTTAG}"
    source  /app/localSetup.sh
fi

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
if [ $? -ne 0 ]; then
  echo gemc executable not found
	exit 1
fi
# for some reason DYLD_LIBRARY_PATH is not passed to this script
export DYLD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${GLIBRARY}/lib

# using the just compiled gemc and the container sci-g
cd $SCIG
./ci/tests.sh -e $detector -t
exit $?

