#!/usr/bin/env bash

# Shared CI image matrix configuration. Keep container and library versions here
# so deploy/test and sanitizer workflows exercise the same base images.

get_geant4_tags() { echo "11.4.1"; } # space separated list.
get_gemc_tags() { echo "dev"; }
get_cpu_architectures() { echo "arm64 amd64"; } # space separated list.

get_runner() {
	local arch=$1
	case "$arch" in
		"arm64") echo "ubuntu-24.04-arm" ;;
		"amd64") echo "ubuntu-latest" ;;
		*)
			echo "ERROR: unsupported arch $arch" >&2
			return 2
			;;
	esac
}

lc() { printf '%s' "$1" | tr '[:upper:]' '[:lower:]'; } # portable lowercasing

OS_VERSIONS=(
  "ubuntu=24.04"
  "ubuntu=26.04"
  "fedora=44"
  "almalinux=10"
  "debian=13"
  "archlinux=latest"
)

get_latest_almalinux() {
  # Highest almalinux version listed in OS_VERSIONS (e.g. 9.4 vs 10 -> 10).
  local pair ver max=""
  for pair in "${OS_VERSIONS[@]}"; do
    [[ "${pair%%=*}" == "almalinux" ]] || continue
    ver="${pair#*=}"
    if [[ -z "$max" ]] || [[ "$(printf '%s\n%s\n' "$max" "$ver" | sort -V | tail -1)" == "$ver" ]]; then
      max="$ver"
    fi
  done
  printf '%s' "$max"
}

build_image_ref() {
  # Owner from env (Actions sets this). Fallback for local runs.
  local owner="${GITHUB_REPOSITORY_OWNER:-gemc}"

  # Repo name = LAST segment of GITHUB_REPOSITORY (strip any "owner/" prefix).
  # Fallback to a default if env is missing during local runs.
  local repo_full="${GITHUB_REPOSITORY:-gemc/src}"
  local repo="${repo_full##*/}" # strip anything up to and including the last slash

  # Lowercase both parts (GHCR requires lowercase)
  printf 'ghcr.io/%s/%s' "$(lc "$owner")" "$(lc "$repo")"
}
