#!/usr/bin/env zsh

# Purpose: compiles gemc with optional sanitizers or debugging options.

# Container run:
# docker run --rm -it  ghcr.io/gemc/g4install:11.4.1-ubuntu-24.04  bash -li
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh

source ci/env.sh
meson_option=$(meson_setup_options $1)

{
  echo " > Geant-config: $(command -v geant4-config) : $(geant4-config --version)"
  echo " > Root-config: $(command -v root-config) : $(root-config --version)"
  echo
  echo " > Meson Interactive Options: $test_interactive_option"
  echo " > Meson Setup Options: $meson_option"
  echo " > Using $jobs cores"
  echo
} | tee -a "$setup_log"


{
  echo
  rm -rf $GEMC
  echo " > Removed $GEMC directory"
  echo " > Running meson setup build $=meson_option"
  meson setup build $=meson_option
} | tee -a $setup_log


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

test_options=(
  -C build
  --print-errorlogs
  -j 1
  --no-rebuild
  --num-processes 1
)

# Specific tests to run when sanitizer is enabled
sanitizer_tests=(
  -v
  api_create_template_system_withG4Box
  api_template_replace_geometry_withG4Box
  api_template_build_geometry_in_test_of_replacing_geometry_withG4Box_with_formatascii
  api_run_gemc_with_replaced_geometry_using_G4Box_with_formatascii
  test_gparticle_double_verbose
  test_event_dispenser_verbose
  examples_geo_basic_simple_flux_ascii_variation_default
)

undefined_preload_tests=(
  test_gstreamer_csv_verbose
)


meson_args=( "${test_options[@]}" )

# if $1 is NOT one of sanitize option, run meson test
case "$1" in
  address|thread|undefined|leak)
    meson_args+=( "${sanitizer_tests[@]}" )
    ;;
  *)
    meson_args+=( -v )
    ;;
esac

: > "$test_log"
echo " > Running meson test with options:"  "${meson_args[@]}" | tee -a "$test_log"
meson test "${meson_args[@]}" >> "$test_log"
if [ $? -ne 0 ]; then
  echo " > Meson Tests failed. Log: "
  cat $test_log
  exit 1
else
  echo " > Meson Tests Successful"
  echo
fi

if [ "$1" = "undefined" ]; then
  gstreamer_csv_plugin="$PWD/build/gstreamer_csv_plugin.gplugin"
  # UBSan on Linux can exhaust static TLS before GEMC's runtime dlopen() loads
  # this plugin. Preloading it makes the dynamic loader reserve TLS at process
  # startup while keeping the workaround scoped to the affected sanitizer test.
  echo " > Running undefined sanitizer preload tests with LD_PRELOAD=$gstreamer_csv_plugin" | tee -a "$test_log"
  LD_PRELOAD="$gstreamer_csv_plugin" meson test "${test_options[@]}" -v "${undefined_preload_tests[@]}" >> "$test_log"
  if [ $? -ne 0 ]; then
    echo " > Meson Tests failed. Log: "
    cat $test_log
    exit 1
  else
    echo " > Meson preload tests successful"
    echo
  fi
fi

echo "   - Successful: $(grep 'Ok:' "$test_log" | awk '{sum += $2} END {print sum + 0}')" | tee -a "$test_log"
echo "   - Failures: $(grep 'Fail:' "$test_log" | awk '{sum += $2} END {print sum + 0}')" | tee -a "$test_log"
echo " > Complete test log: $test_log"
