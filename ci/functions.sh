#!/usr/bin/env zsh

# if we are in the docker container, we need to load the modules
if [[ -z "${AUTOBUILD}" ]]; then
    echo "\nNot in container"
else
    echo "\n > Running In a Docker Container"
    source  /etc/profile.d/localSetup.sh
    # In github actions, the clone is not deep, so we need to fetch the tags
    git fetch --prune --unshallow --tags
fi

function set_environment {
  export GEMC=$SIM_HOME/gemc
  mkdir -p $GEMC
}

function set_ld_path {
  if [ -d $GEMC/lib ]; then
    export LD_LIBRARY_PATH=$GEMC/lib:$LD_LIBRARY_PATH
  elif [ -d $GEMC/lib64 ]; then
    export LD_LIBRARY_PATH=$GEMC/lib64:$LD_LIBRARY_PATH
  fi
}

function show_installation {
  echo
  echo "- Content of $GEMC"
  ls -lrt $GEMC
  echo "- Content of $GEMC/bin"
  ls -lrt $GEMC/bin

  if [ -d $GEMC/lib ]; then
    echo "- Content of $GEMC/lib"
    ls -lrt $GEMC/lib
  elif [ -d $GEMC/lib64 ]; then
    echo "- Content of $GEMC/lib64"
    ls -lrt $GEMC/lib64
  fi
}
