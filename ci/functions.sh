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
module load gemc/dev3
mkdir -p $GEMC


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
