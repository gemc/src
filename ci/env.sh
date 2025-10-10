#!/usr/bin/env zsh

DetectorDirNotExisting() {
	echo "System directory: $system not existing"
	exit 3
}

function show_installation {
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

function meson_options {
    # valid options: address, thread, undefined, memory, leak

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

    echo $meson_options $buildtype
}


	# recent versions of Git refuse to touch a repository whose on-disk owner
	# doesn’t match the UID that is running the command
	# mark the workspace (and any nested path) as safe
	echo "Marking workspace as safe for Git"
	git config --global --add safe.directory '*'

is_shallow=$(git rev-parse --is-shallow-repository)

if [ "$is_shallow" = "true" ]; then
  echo "Repository is_shallow: $is_shallow"
  echo " > Fetching all tags"
  git fetch --tags --unshallow
fi

# if we are in the docker container, we need to load the modules
if [[ -z "${AUTOBUILD}" ]]; then
	echo "\nNot in container"
else
	echo "\nIn docker container."
	if [[ -n "${GITHUB_WORKFLOW}" ]]; then
		echo "GITHUB_WORKFLOW: ${GITHUB_WORKFLOW}"
	fi
	source /etc/profile.d/local_g4setup.sh
	echo
fi

export ARTIFACT_DIR=$RUNNER_TEMP/artifacts
mkdir -p $ARTIFACT_DIR
