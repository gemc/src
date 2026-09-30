#!/usr/bin/env bash

# Read-only diagnostics shared by build.sh and workflows that run an existing GEMC installation.
report_environment() {
  local cpu_count cpu_model topology affinity cpu_info physical logical sockets config_path data_dir

  cpu_count=$(nproc 2>/dev/null || getconf _NPROCESSORS_ONLN 2>/dev/null || printf 'unknown')
  cpu_model=unknown
  topology=unknown
  affinity='not available'

  if [[ $(uname -s) == Darwin ]]; then
    cpu_model=$(sysctl -n machdep.cpu.brand_string 2>/dev/null || printf 'unknown')
    physical=$(sysctl -n hw.physicalcpu 2>/dev/null)
    logical=$(sysctl -n hw.logicalcpu 2>/dev/null)
    sockets=$(sysctl -n hw.packages 2>/dev/null || printf '1')
    if [[ $physical -gt 0 && $logical -gt 0 ]]; then
      topology="$physical physical cores, $((logical / physical)) threads/core, $sockets socket(s)"
    fi
  elif command -v lscpu >/dev/null 2>&1; then
    cpu_info=$(LC_ALL=C lscpu 2>/dev/null)
    cpu_model=$(printf '%s\n' "$cpu_info" | awk -F: '/^Model name:/ {
      sub(/^[^:]*:[[:space:]]*/, ""); print; exit
    }')
    topology=$(printf '%s\n' "$cpu_info" | awk -F: '
      /^Core\(s\) per socket:/ { cores = $2 + 0 }
      /^Thread\(s\) per core:/ { threads = $2 + 0 }
      /^Socket\(s\):/ { sockets = $2 + 0 }
      END {
        if (cores && threads && sockets)
          printf "%d physical cores, %d threads/core, %d socket%s", \
            cores * sockets, threads, sockets, (sockets == 1 ? "" : "s")
        else
          print "unknown"
      }')
  elif [[ -r /proc/cpuinfo ]]; then
    cpu_model=$(awk -F: '/^(model name|Hardware)[[:space:]]*:/ {
      sub(/^[^:]*:[[:space:]]*/, ""); print; exit
    }' /proc/cpuinfo)
  fi

  if command -v taskset >/dev/null 2>&1; then
    affinity=$(LC_ALL=C taskset -pc "$$" 2>/dev/null || printf 'not available')
  fi

  printf 'CPUs available: %s\n' "$cpu_count"
  printf 'CPU: %s\n' "${cpu_model:-unknown}"
  printf 'Topology: %s\n' "$topology"
  printf 'Affinity: %s\n' "$affinity"

  if [[ -n ${GEMC_DOCKER_IMAGE:-} ]]; then
    printf 'Docker image: %s\n' "$GEMC_DOCKER_IMAGE"
  elif [[ -f /.dockerenv || -n ${AUTOBUILD:-} ]]; then
    printf 'Docker image: unknown (GEMC_DOCKER_IMAGE is not set)\n'
  else
    printf 'Docker image: not running in Docker\n'
  fi

  if config_path=$(command -v geant4-config); then
    printf 'geant4-config: %s\n' "$config_path"
    printf 'Geant4 version: %s\n' "$(geant4-config --version 2>/dev/null || printf 'unavailable')"
  else
    printf 'geant4-config: not found\n'
  fi

  if [[ -n ${G4INSTALL:-} ]]; then
    data_dir="$G4INSTALL/share/Geant4/data"
    printf 'Geant4 data directory: %s\n' "$data_dir"
    if [[ -d $data_dir ]]; then
      LC_ALL=C ls -la "$data_dir" || printf 'Geant4 data directory could not be listed\n'
    else
      printf 'Geant4 data directory: not found\n'
    fi
  else
    printf 'Geant4 data directory: unavailable (G4INSTALL is not set)\n'
  fi
}

report_environment
