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
    echo "- Content of \$GEMC=$GEMC" 
    ls -lrt $GEMC 

    echo "- Content of \$GEMC/bin=$GEMC/bin" 
    ls -lrt $GEMC/bin 

    if [ -d $GEMC/lib ]; then
      echo "- Content of \$GEMC/lib=$GEMC/lib" 
      ls -lrt $GEMC/lib 
    fi

    echo " ldd of $GEMC/bin/gemc:" 

    # if on unix, use ldd , if on mac, use otool -L
    if [[ "$(uname)" == "Darwin" ]]; then
      otool -L $GEMC/bin/gemc 
    else
      ldd $GEMC/bin/gemc 
    fi

    echo " > To check gemc installation:  cat $gemc_install_show"
  } | tee -a  $gemc_install_show

}

function meson_setup_options {
    # valid options: address, thread, undefined, memory, leak
    local install_dir="${GEMC:?GEMC not set}"

    meson_options=""
    buildtype=" -Dbuildtype=debug "

    case $1 in
        "address")
            meson_options="-Db_sanitize=address"
            ;;
        "thread")
            meson_options="-Db_sanitize=thread"
            ;;
        "undefined")
            meson_options="-Db_sanitize=undefined"
            ;;
        "memory")
            meson_options="-Db_sanitize=memory"
            ;;
        "leak")
            meson_options="-Db_sanitize=leak"
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

    args=(
            $meson_options
            $test_interactive_option
            "--native-file=core.ini"
            "-Droot=enabled"
            "-Dprefix=${install_dir}"
            "-Dpkg_config_path=$PKG_CONFIG_PATH:${install_dir}/lib/pkgconfig"
            $buildtype
        )

    echo "${args[@]}"
}

log_dir=$SIM_HOME/logs
test_interactive_option=""

# if we are in the docker container, we need to load the modules
if [[ -z "${AUTOBUILD}" ]]; then
	echo "\nNot in container"
  test_interactive_option="-Di_test=true"
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


export GEMC=${SIM_HOME}/gemc/dev


# detect cores and cap at 16
cores=$(getconf _NPROCESSORS_ONLN 2>/dev/null || nproc)
jobs=$((cores < 16 ? cores : 16))

mkdir -p $log_dir
setup_log=$log_dir/setup.log
compile_log=$log_dir/build.log
install_log=$log_dir/install.log
test_log=$log_dir/test.log
gemc_install_show=$log_dir/gemc.log

touch $setup_log $compile_log $install_log $test_log
echo
