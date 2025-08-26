#!/usr/bin/env zsh

# Purpose: compiles gemc with optional sanitizers or debugging options.

# Container run:
# docker run -it --rm --platform linux/amd64 jeffersonlab/geant4:g4v11.3.2-almalinux94 sh
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh

source ci/env.sh

# module gemc gives $GEMC (used in meson prefix) and PKG_CONFIG_PATH
# not strickly necessary, one could set those manually
module load gemc/dev3
echo GEMC for prefix set to: $GEMC
echo

meson_option=$(meson_options $1)
max_threads=$(max_j)

mkdir -p /root/src/logs
setup_log=/root/src/logs/setup.log
compile_log=/root/src/logs/build.log
test_log=/root/src/logs/test.log

touch $setup_log $compile_log $test_log

echo Logs:

ls -l $setup_log
ls -l $compile_log
ls -l $test_log


echo " > Geant-config: $(which geant4-config) : $(geant4-config --version)" > $setup_log
echo " > Root-config: $(which root-config) : $(root-config --version)" >> $setup_log

setup_options=" --native-file=core.ini $meson_option -Dprefix=$GEMC --wipe "

echo " > Running build Configure with setup build options: $setup_options"  >> $setup_log
meson setup build $=setup_options  >> $setup_log
if [ $? -ne 0 ]; then
	echo "Build Configure failed. Log: "
	cat $setup_log
  exit 1
else
  echo Build Configure Successful
  echo ; echo
fi

cd build  || exit 1

echo " > Running meson compile -v  -j $max_threads"  > $compile_log
meson compile -v  -j $max_threads >> $compile_log
if [ $? -ne 0 ]; then
	echo "Compile failed. Log: "
	cat $compile_log
  exit 1
else
  echo Compile Successful
  echo ; echo
fi


echo " > Current directory: $(pwd) content:"  >> $compile_log
ls -l  >> $compile_log

echo " > Running meson install"  >> $compile_log
meson install   >> $compile_log
if [ $? -ne 0 ]; then
	echo "Install failed. Log: "
	cat $compile_log
  exit 1
else
  echo Install Successful
  echo ; echo
fi

echo " > $GEMC recursive content:"  >> $compile_log
ls -lR $GEMC  >> $compile_log

# if $1 is NOT one of sanitize option, run meson test
if [[ $1 != @(address|thread|undefined|memory|leak) ]]; then
    echo " > Running meson test" > $test_log
    meson test  -j 1 --print-errorlogs --no-rebuild  >>  $test_log
    if [ $? -ne 0 ]; then
    	echo "Test failed. Log: "
    	cat $test_log
      exit 1
    else
      echo Install Successful
      echo ; echo
    fi
fi

echo
echo " ldd of $GEMC/bin/gemc:" >> $compile_log | tee -a

# if on unix, use ldd , if on mac, use otool -L
if [[ "$(uname)" == "Darwin" ]]; then
  otool -L $GEMC/bin/gemc
else
  ldd $GEMC/bin/gemc >> $compile_log | tee -a
fi

