#!/usr/bin/env zsh

# Purpose: compiles gemc with optional sanitizers or debugging options.

# Container run:
# docker run -it --rm --platform linux/amd64 ghcr.io/gemc/g4install:11.3.2-archlinux-latest bash -il
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh

source ci/env.sh

# module gemc gives $GEMC (used in meson prefix) and PKG_CONFIG_PATH
# not strickly necessary, one could set those manually
module load gemc/dev
echo GEMC for prefix set to: $GEMC
echo

meson_option=$(meson_options $1)

mkdir -p /root/src/logs
setup_log=/root/src/logs/setup.log
compile_log=/root/src/logs/build.log
test_log=/root/src/logs/test.log

touch $setup_log $compile_log $test_log

echo Logs:

ls -l $setup_log
ls -l $compile_log
ls -l $test_log

echo " > Geant-config: $(which geant4-config) : $(geant4-config --version)" | tee $setup_log
echo " > Root-config: $(which root-config) : $(root-config --version)" | tee -a $setup_log

setup_options=" --native-file=core.ini $meson_option -Dprefix=$GEMC --wipe "

local mode="${1:-release}"
local install_dir="${GEMC:?GEMC not set}"

local args=(
  "--native-file=core.ini"
  "-Droot=enabled"
  "-Dprefix=${install_dir}"
)

# detect cores and cap at 32
cores=$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc)
jobs=$((cores < 32 ? cores : 32))

echo " > Applying patch to version 0.8.0" | tee -a $setup_log
meson subprojects update yaml-cpp --reset

echo " > Running build Configure with setup build options: ${args[@]}" | tee -a $setup_log
meson setup build "${args[@]}" --wipe >>$setup_log
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
meson compile -C build -v -j $cores >> $compile_log
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
