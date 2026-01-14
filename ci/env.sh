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
  echo
  echo "- Content of \$GEMC=$GEMC"
  ls -lrt $GEMC

  echo "- Content of \$GEMC/bin=$GEMC/bin"
  ls -lrt $GEMC/bin

  if [ -d $GEMC/lib ]; then
    echo "- Content of \$GEMC/lib=$GEMC/lib"
    ls -lrt $GEMC/lib
  fi
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

    additional_args=(
                    		"--native-file=core.ini"
                    		"-Di_test=true"
                    		"-Droot=enabled"
                    		"-Dprefix=${install_dir}"
                    		"-Dpkg_config_path=$PKG_CONFIG_PATH:${install_dir}/lib/pkgconfig"
                    	)

    echo $meson_options $buildtype "${additional_args[@]}"
}


# recent versions of Git refuse to touch a repository whose on-disk owner
# doesn’t match the UID that is running the command
# mark the workspace (and any nested path) as safe
echo "Marking workspace as safe for Git"
git config --global --add safe.directory '*'

enable_git_describe
log_dir=$SIM_HOME/logs

# if we are in the docker container, we need to load the modules
if [[ -z "${AUTOBUILD}" ]]; then
	echo "\nNot in container"
else
	echo "\nIn docker container."
	log_dir=/root/src/logs
fi

export $GEMC=$SIM_HOME/gemc/dev

setup_log=/root/src/logs/setup.log
compile_log=/root/src/logs/build.log
test_log=/root/src/logs/test.log

touch $setup_log $compile_log $test_log

echo Logs:

ls -l $setup_log
ls -l $compile_log
ls -l $test_log