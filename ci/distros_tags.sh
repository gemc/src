#!/usr/bin/env bash
set -euo pipefail

get_geant4_tag() { echo "11.3.2"; }
get_gemc_tag() { echo "dev"; }

# Single source of truth (order preserved)
OS_VERSIONS=(
  "ubuntu=24.04"
  "fedora=40"
  "almalinux=9.4"
  "debian=12"
  "archlinux=latest"
)

build_matrix() {
  local g4 gemc
  g4="$(get_geant4_tag)"
  gemc="$(get_gemc_tag)"

  local body="" sep="" pair os ver baseimage img
  for pair in "${OS_VERSIONS[@]}"; do
    os="${pair%%=*}"
    ver="${pair#*=}"
    baseimage="${g4}-${os}-${ver}"
    img="${os}-${ver}" # don't use the name 'image' here to avoid clobbering caller vars
    body+=${sep}"{\"osname\":\"${os}\",\"baseimage\":\"${baseimage}\",\"image\":\"${img}\",\"osversion\":\"${ver}\",\"gemc_tag\":\"${gemc}\",\"geant4_tag\":\"${g4}\"}"
    sep=","
  done

  local json="{\"include\":[${body}]}"
  if command -v jq >/dev/null 2>&1; then
    printf '%s' "$json" | jq .
  else
    printf '%s' "$json"
  fi
}

# portable lowercasing
lc() { printf '%s' "$1" | tr '[:upper:]' '[:lower:]'; }

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

main() {
  local image_ref
  image_ref="$(build_image_ref)"

  if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
    local DELIM="MATRIX_$(date +%s%N)"
    {
      echo "matrix<<$DELIM"
      build_matrix
      echo "$DELIM"
      echo "image=$image_ref"
    } >>"$GITHUB_OUTPUT"
  else
    # Local run: only print the matrix (no trailing image line)
    build_matrix
    echo
    echo "images located at: $image_ref"
  fi
}

main "$@"
