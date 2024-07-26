#!/usr/bin/env zsh

# if we are in the docker container, we need to load the modules
if [[ -z "${AUTOBUILD}" ]]; then
    echo "\nNot in container"
else
    echo "\n > Running In a Docker Container"
    source  /etc/profile.d/localSetup.sh
fi

# If shallow, need to fetch the tags
if $(git rev-parse --is-shallow-repository); then
    git fetch --prune --unshallow --tags
fi

# assuming only geant4 and not gemc is loaded
echo " > Setting environment, current dir: $(pwd)"
ls -l
module load sim_system
echo " > SIM_HOME is $SIM_HOME"
export GEMC=$SIM_HOME/gemc
export PYTHONPATH=${PYTHONPATH}:${GEMC}/api
mkdir -p $GEMC

function set_ld_path {
  echo " > Setting LD Path"
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
