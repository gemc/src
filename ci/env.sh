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

# If shallow, need to fetch the tags
if $(git rev-parse --is-shallow-repository); then
    git fetch --prune --unshallow --tags
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
mkdir -p $GEMC

function sanitize_options {
    if [ "$1" == "leak" ]; then
        sanitize_option="-Db_sanitize=address,undefined"
    elif [ "$1" == "thread" ]; then
        sanitize_option="-Db_sanitize=thread"
    else
        sanitize_option=""
    fi
}