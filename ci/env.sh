#!/usr/bin/env zsh

# enable git describe --tags, needed by meson
function enable_git_describe {
  is_shallow=$(git rev-parse --is-shallow-repository)
  if [ "$is_shallow" = "true" ]; then
    echo "Repository is_shallow: $is_shallow"
    echo " > Fetching all tags"
    git fetch --tags --unshallow
  fi
}

function show_gemc_installation {
  {
  	local install_dir="${SIM_HOME:?SIM_HOME not set}/gemc/dev"

    echo "- Content of $install_dir=$install_dir"
    ls -lrt $install_dir

    echo "- Content of \$install_dir/bin=$install_dir/bin"
    ls -lrt $install_dir/bin

    if [ -d $install_dir/lib ]; then
      echo "- Content of \$install_dir/lib=$install_dir/lib"
      ls -lrt $install_dir/lib
    fi

    echo " ldd of $install_dir/bin/gemc:"

    # if on unix, use ldd , if on mac, use otool -L
    if [[ "$(uname)" == "Darwin" ]]; then
      otool -L $install_dir/bin/gemc
    else
      ldd $install_dir/bin/gemc
    fi

    echo " > To check gemc installation:  cat $gemc_install_show"
  } | tee -a  $gemc_install_show

}

function meson_setup_options {
    # valid options: address, thread, undefined, memory, leak
    local install_dir="${SIM_HOME:?SIM_HOME not set}/gemc/dev"

    meson_options=""
    sanitizer_library_options="-Ddefault_library=shared -Ddefault_both_libraries=shared"
    buildtype=" -Dbuildtype=debug "
    local pkg_config_path="${install_dir}/lib/pkgconfig"

    case ${1:-} in
        "address")
            meson_options="-Db_sanitize=address $sanitizer_library_options"
            ;;
        "thread")
            meson_options="-Db_sanitize=thread $sanitizer_library_options"
            ;;
        "undefined")
            meson_options="-Db_sanitize=undefined $sanitizer_library_options"
            ;;
        "memory")
            meson_options="-Db_sanitize=memory $sanitizer_library_options"
            ;;
        "leak")
            meson_options="-Db_sanitize=leak $sanitizer_library_options"
            ;;
        "profile")
            meson_options=""
            ;;
        "debug")
            meson_options=""
            buildtype=" -Dbuildtype=debug "
            ;;
        *)
            meson_options=""
            buildtype=" -Dbuildtype=release "
            ;;
    esac

    if [[ -n "${PKG_CONFIG_PATH:-}" ]]; then
        pkg_config_path="${pkg_config_path}:${PKG_CONFIG_PATH}"
    fi

    args=(
            $meson_options
            $test_interactive_option
            "--native-file=core.ini"
            "-Droot=enabled"
            "-Dprefix=${install_dir}"
            "-Dpkg_config_path=${pkg_config_path}"
            $buildtype
        )

    echo "${args[@]}"
}

# Run any command, retrying on failure. Appends output to $test_log.
# The number of attempts is GEMC_TEST_MAX_ATTEMPTS (default 3), so a command is
# re-run up to that many times until it succeeds.
function run_command_with_retry {
  local label="$1"
  shift

  local max_attempts="${GEMC_TEST_MAX_ATTEMPTS:-3}"

  for (( attempt = 1; attempt <= max_attempts; attempt++ )); do
    echo " > Running ${label} (attempt ${attempt}/${max_attempts}):" "$@" | tee -a "$test_log"
    "$@" >> "$test_log"
    local exit_code=$?

    if [ $exit_code -eq 0 ]; then
      return 0
    fi

    if [ $attempt -eq $max_attempts ]; then
      return $exit_code
    fi

    echo " > ${label} failed; retrying (attempt $((attempt + 1))/${max_attempts})" | tee -a "$test_log"
  done
}

export GEMC="${SIM_HOME:?SIM_HOME not set}/gemc"

log_dir=$SIM_HOME/logs
test_interactive_option=""

# if we are in the docker container, we need to load the modules
if [[ -z "${AUTOBUILD:-}" ]]; then
	echo "\nNot in container"
  # Local developer machines run the interactive (GUI) tests by default.
  # Headless CI (e.g. the macOS tarball workflow) opts out with GEMC_INTERACTIVE_TESTS=false.
  if [[ "${GEMC_INTERACTIVE_TESTS:-true}" == "true" ]]; then
    test_interactive_option="-Di_test=true"
  fi
else
	echo "\nIn docker container."
  # recent versions of Git refuse to touch a repository whose on-disk owner
  # doesn’t match the UID that is running the command
  # mark the workspace (and any nested path) as safe
  echo "Marking workspace as safe for Git"
  git config --global --add safe.directory '*'
  enable_git_describe
	log_dir=/root/src/logs
fi

# detect cores and cap at 16
cores=$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc)
jobs=$((cores < 16 ? cores : 16))

mkdir -p $log_dir
setup_log=$log_dir/01_setup.log
compile_log=$log_dir/02_build.log
install_log=$log_dir/03_install.log
gemc_install_show=$log_dir/04_show_install.log
test_log=$log_dir/05_tests.log

touch $setup_log $compile_log $install_log $test_log
echo
