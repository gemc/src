#!/usr/bin/env zsh

# Purpose: compiles gemc with optional sanitizers or debugging options.

# Container run:
# docker run -it --rm --platform linux/amd64 ghcr.io/gemc/g4install:11.3.2-archlinux-latest bash -il
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh

source ci/env.sh
meson_option=$(meson_options $1)

echo " > Geant-config: $(which geant4-config) : $(geant4-config --version)" | tee $setup_log
echo " > Root-config: $(which root-config) : $(root-config --version)" | tee -a $setup_log
echo Meson Options: $meson_option
exit

# detect cores and cap at 16
cores=$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc)
jobs=$((cores < 16 ? cores : 16))

meson subprojects download yaml-cpp
meson subprojects download assimp
echo " > Applying patch to version 0.8.0" | tee -a $setup_log
meson subprojects update yaml-cpp --reset
echo
echo " > Running meson setup build ${args[@]}" | tee -a $setup_log
meson setup build "meson_option" - >> $setup_log
if [ $? -ne 0 ]; then
  echo "Build Configure failed. Log: "
  cat $setup_log
  exit 1
else
  echo Build Configure Successful
  echo
  echo
fi

echo " > Running meson install -v  -j $cores" | tee $compile_log
meson compile -C build -v -j $jobs >> $compile_log
meson install -C build  >> $compile_log
if [ $? -ne 0 ]; then
  echo "Compile or Install failed. Log: "
  cat $compile_log
  exit 1
else
  echo Compile Successful
  echo
  echo
fi

echo " > $GEMC recursive content:" | tee -a $compile_log
ls -lR $GEMC >>$compile_log

# if $1 is NOT one of sanitize option, run meson test
if [[ $1 != @(address|thread|undefined|memory|leak) ]]; then
  echo " > Running meson test" | tee $test_log
  meson test -C build -j 1 --print-errorlogs --no-rebuild --num-processes 1 >>$test_log
  echo Successful: $(cat $test_log | grep "Ok:" | awk '{print $2}')
  echo Failures: $(cat $test_log | grep "Fail:" | awk '{print $2}')
fi

echo
echo " ldd of $GEMC/bin/gemc:" | tee -a $compile_log

# if on unix, use ldd , if on mac, use otool -L
if [[ "$(uname)" == "Darwin" ]]; then
  otool -L $GEMC/bin/gemc
else
  ldd $GEMC/bin/gemc | tee -a $compile_log
fi
