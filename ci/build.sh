#!/usr/bin/env zsh

# Purpose: compiles gemc with optional sanitizers or debugging options.

# Container run:
# docker run --rm -it  ghcr.io/gemc/g4install:11.4.2-ubuntu-24.04  bash -li
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh

# Load environment variables: log file paths, job count, helper functions.
source ci/env.sh

# Resolve meson setup flags from the optional sanitizer argument.
meson_option=$(meson_setup_options "${1:-}")

if command -v geant4-config >/dev/null 2>&1; then
  geant4_found=true
  geant4_summary="$(command -v geant4-config) : $(geant4-config --version)"
else
  geant4_found=false
  geant4_summary="not found"
fi

# Print build environment summary (compiler paths, options, core count).
{
  echo " > Geant-config: $geant4_summary"
  echo " > Root-config: $(command -v root-config) : $(root-config --version)"
  echo
  echo " > Meson Interactive Options: $test_interactive_option"
  echo " > Meson Setup Options: $meson_option"
  echo " > Using $jobs cores"
  echo
} | tee -a "$setup_log"

if [[ "$geant4_found" == false ]]; then
  echo "Geant4 was not found. Load a Geant4 module before configuring GEMC." | tee -a "$setup_log"
  exit 1
fi


# Remove any previous install directory, then configure the build tree.
{
  echo
  rm -rf $GEMC
  echo " > Removed $GEMC directory"
  echo " > Running meson setup build $=meson_option"
  meson setup build $=meson_option
} | tee -a $setup_log

# $? reflects tee's exit code in a pipeline; use ${pipestatus[1]} for meson's.
if [ ${pipestatus[1]} -ne 0 ]; then
  echo " > Meson Configure failed. Log: "
  cat $setup_log
  exit 1
else
  echo " > Meson Configure Successful"
  echo
fi


# Compile all targets using all available cores.
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

# Install built artifacts to the configured prefix.
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

# Print a summary of the installed gemc files and version.
show_gemc_installation

# Base options shared by all meson test invocations.
test_options=(
  -C build
  --print-errorlogs
  -j 1
  --no-rebuild
  --num-processes 1
)

# Subset of tests run when a sanitizer is active (keeps the suite fast and focused).
sanitizer_tests=(
  -v
  test_generator_lund_file_events
  test_gparticle_double_verbose
  test_event_dispenser_verbose
  examples_geo_basic_simple_flux_ascii_variation_default
)

# Tests that need the CSV plugin preloaded to work under UBSan.
undefined_preload_tests=(
  test_gstreamer_csv_verbose
)

# Start with the base test options; append sanitizer-specific or verbose flag below.
meson_args=( "${test_options[@]}" )

# When a sanitizer is active run only the targeted subset; otherwise run all tests verbosely.
case "$1" in
  address|thread|undefined|leak)
    meson_args+=( "${sanitizer_tests[@]}" )
    ;;
  *)
    meson_args+=( -v )
    ;;
esac

# Clear (or create) the test log before the first test run.
> "$test_log"
if ! run_command_with_retry "meson test" meson test "${meson_args[@]}"; then
  echo " > Meson Tests failed. Log: "
  cat $test_log
  exit 1
else
  echo " > Meson Tests Successful"
  echo
fi

# Under UBSan, preload the CSV plugin to avoid static-TLS exhaustion at dlopen time.
if [ "${1:-}" = "undefined" ]; then
  gstreamer_csv_plugin="$PWD/build/gstreamer_csv_plugin.gplugin"
  # UBSan on Linux can exhaust static TLS before GEMC's runtime dlopen() loads
  # this plugin. Preloading it makes the dynamic loader reserve TLS at process
  # startup while keeping the workaround scoped to the affected sanitizer test.
  if ! run_command_with_retry "undefined sanitizer preload tests" env LD_PRELOAD="$gstreamer_csv_plugin" \
    meson test "${test_options[@]}" -v "${undefined_preload_tests[@]}"; then
    echo " > Meson Tests failed. Log: "
    cat $test_log
    exit 1
  else
    echo " > Meson preload tests successful"
    echo
  fi
fi

# Print a pass/fail summary parsed from the accumulated test log.
echo "   - Successful: $(grep 'Ok:' "$test_log" | awk '{sum += $2} END {print sum + 0}')" | tee -a "$test_log"
echo "   - Failures: $(grep 'Fail:' "$test_log" | awk '{sum += $2} END {print sum + 0}')" | tee -a "$test_log"
echo " > Complete test log: $test_log"
