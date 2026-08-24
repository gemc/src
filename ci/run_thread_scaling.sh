#!/usr/bin/env bash

# Run one installed GEMC example in an isolated directory for ThreadScale.
set -euo pipefail

if (( $# != 6 )); then
  echo "Usage: $0 INSTALL_ROOT EXAMPLE EVENTS THREADS RUN REPLICA" >&2
  exit 2
fi

install_root="$1"
example="$2"
events="$3"
threads="$4"
run="$5"
replica="$6"

for value in "${events}" "${threads}" "${run}" "${replica}"; do
  if [[ ! "${value}" =~ ^[0-9]+$ ]]; then
    echo "Event, thread, run, and replica values must be non-negative integers: ${value}" >&2
    exit 2
  fi
done
if (( events < 1 || threads < 1 || replica < 1 )); then
  echo "Event, thread, and replica values must be positive" >&2
  exit 2
fi

example_name="${example##*/}"
yaml="${install_root}/examples/${example}/${example_name}.yaml"
database="${install_root}/examples/gemc.db"
if [[ ! -x "${install_root}/bin/gemc" || ! -f "${yaml}" || ! -f "${database}" ]]; then
  echo "Incomplete GEMC thread-scaling installation at ${install_root}" >&2
  exit 1
fi

export DOCKER_ENTRYPOINT_SOURCE_ONLY=1
# shellcheck source=/dev/null
source /usr/local/bin/docker-entrypoint.sh
module load geant4/"${GEANT4_TAG:?GEANT4_TAG is not set}"

export PATH="${install_root}/bin:${PATH}"
export LD_LIBRARY_PATH="${install_root}/lib:${LD_LIBRARY_PATH:-}"

scratch="${RUNNER_TEMP:?RUNNER_TEMP is not set}/gemc-thread-scaling/${example_name}"
scratch="${scratch}/replica-${replica}/threads-${threads}/run-${run}"
mkdir -p "${scratch}"
cd "${scratch}"

exec "${install_root}/bin/gemc" "${yaml}" \
  "-sql=${database}" \
  "-n=${events}" \
  "-nthreads=${threads}" \
  -seed=12345 \
  '-gstreamer=[]'
