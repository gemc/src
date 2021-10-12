#!/usr/bin/bash -e

# GEMC Continuous Integration
# ----------------------------
#
# Original Instructions:
# https://docs.github.com/en/actions/creating-actions/creating-a-docker-container-action
# The steps on "main" are pasted on the Github page under "Actions" > set up a workflow yourself
#
# To debug this on the container:
#
# cp ci.sh ~/mywork
# docker run -it --rm -v ~/mywork:/jlab/work/mywork jeffersonlab/gemc:3.0 bash
# cd work/mywork
# ./ci.sh

# load environment
source /etc/profile.d/jlab.sh

echo
echo "GEMC Validation: $1"
echo
time=$(date)
echo "::set-output name=time::$time"

cd /root
git clone http://github.com/gemc/src
cd src


# getting number of available CPUS
copt=" -j"`getconf _NPROCESSORS_ONLN`" OPT=1"
echo
echo Compiling echo Compiling GEMC with options: "$copt"
scons $copt

# checking on various libraries
ls gemc
if [ $? -ne 0 ]; then
  echo compilation not completed
  exit 1
fi
