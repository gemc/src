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
  if [ -d $GEMC/lib64 ]; then
    echo "- Content of \$GEMC/lib64=$GEMC/lib64"
    ls -lrt $GEMC/lib64
  fi
}

function sanitize_options {
    # valid options: address, thread, undefined, memory, leak

    sanitize_option=""
    pgo=""
    buildtype=" -Dbuildtype=debug "

    case $1 in
        "address")
            sanitize_option="-Db_sanitize=address"
            ;;
        "thread")
            sanitize_option="-Db_sanitize=thread"
            ;;
        "undefined")
            sanitize_option="-Db_sanitize=undefined"
            ;;
        "memory")
            sanitize_option="-Db_sanitize=memory"
            ;;
        "leak")
            sanitize_option="-Db_sanitize=leak"
            ;;
        "none")
            sanitize_option=""
            ;;
        *)
            sanitize_option=""
            ;;
    esac

    # if on ubuntu, use pgo generate
    if [ -f /etc/os-release ]; then
        if grep -q "Ubuntu" /etc/os-release; then
            pgo=" -Db_pgo=generate "
        fi
    fi

    echo $sanitize_option $pgo $buildtype
}

is_shallow=$(git rev-parse --is-shallow-repository)
echo "Repository is_shallow: $is_shallow"

if [ "$is_shallow" = "true" ]; then
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
	source /etc/profile.d/localSetup.sh
	module load gemc/dev3
	echo
fi
mkdir -p $GEMC/lib/pkgconfig

# if $GEMC/lib/pkgconfig/geant4.pc is not existing, run meson/install_geant4_root_pkgconfig.py
if [ ! -f $GEMC/lib/pkgconfig/geant4.pc ]; then
    echo " > Running meson/install_geant4_root_pkgconfig.py"
    python3 meson/install_geant4_root_pkgconfig.py
fi

