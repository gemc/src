#!/usr/bin/env zsh

# Purpose: compiles gemc with optional sanitizers or debugging options.

# Container run:
# docker run --rm -it  ghcr.io/gemc/g4install:11.4.0-fedora-40  bash -li
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh

source ci/env.sh
meson_option=$(meson_setup_options $1)

echo " > Geant-config: $(which geant4-config) : $(geant4-config --version)" | tee $setup_log
echo " > Root-config: $(which root-config) : $(root-config --version)" | tee -a $setup_log
echo " > Meson Interactive Options: $test_interactive_option"
echo " > Meson Setup Options: $meson_option"
echo " > Using $jobs cores"
echo

meson subprojects download yaml-cpp
meson subprojects download assimp
echo " > Applying patch to version 0.8.0" | tee -a $setup_log
meson subprojects update yaml-cpp --reset
echo
echo " > Running meson setup build $=meson_option" | tee -a $setup_log
meson setup build $=meson_option >> $setup_log
if [ $? -ne 0 ]; then
  echo " > Meson Configure failed. Log: "
  cat $setup_log
  exit 1
else
  echo " > Meson Configure Successful"
  echo
fi


echo " > Running meson compile -C build -v -j $jobs " | tee -a $compile_log
meson compile -C build -v -j $jobs  >> $compile_log
if [ $? -ne 0 ]; then
  echo " > Meson Compile failed. Log: "
  cat $compile_log
  exit 1
else
  echo " > Meson Compile Successful"
  echo
fi

echo " > Running meson install -C build " | tee -a $install_log
meson install -C build  >> $install_log
if [ $? -ne 0 ]; then
  echo " > Meson Install failed. Log: "
  cat $install_log
  exit 1
else
  echo " > Meson Install Successful"
  echo
fi

show_gemc_installation

# if $1 is NOT one of sanitize option, run meson test
if [[ $1 != @(address|thread|undefined|leak) ]]; then
  echo " > Running meson test with options  -C build -j 1 --print-errorlogs --no-rebuild --num-processes 1 " | tee $test_log
  meson test -C build -v -j 1 --print-errorlogs --no-rebuild --num-processes 1 >> $test_log
  echo "   - Successful: $(cat $test_log | grep "Ok:" | awk '{print $2}')"
  echo "   - Failures: $(cat $test_log | grep "Fail:" | awk '{print $2}')"
  echo " > Complete test log: $test_log"
fi
