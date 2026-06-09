#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$script_dir/tags_config.sh"

print_json() {
  local json=$1

  if command -v jq >/dev/null 2>&1; then
    printf '%s' "$json" | jq -c .
  else
    printf '%s' "$json"
  fi
}

get_sanitizers() {
  local image=$1
  local choice_of_sanitizer="undefined"

  case "$image" in
    "ubuntu" | "fedora" | "almalinux" | "debian" | "archlinux") echo "$choice_of_sanitizer" ;;
    *)
      echo "ERROR: unsupported image $image" >&2
      return 2
      ;;
  esac
}

build_matrix_build() {
  local g4_list arch_list gemc_list
  g4_list="$(get_geant4_tags)"
  arch_list="$(get_cpu_architectures)"
  gemc_list="$(get_gemc_tags)"

  local -a g4_tags arch_tags gemc_tags
  read -r -a g4_tags <<<"$g4_list"
  read -r -a arch_tags <<<"$arch_list"
  read -r -a gemc_tags <<<"$gemc_list"

  local body="" sep="" pair os ver
  for g4v in "${g4_tags[@]}"; do
    for cpuv in "${arch_tags[@]}"; do
      for gemcv in "${gemc_tags[@]}"; do
        local platform runner suffix logs_dir
        platform="linux/$cpuv"
        runner="$(get_runner "$cpuv")"
        suffix="-$cpuv"
        logs_dir="logs-$cpuv"

        for pair in "${OS_VERSIONS[@]}"; do
          os="${pair%%=*}"
          ver="${pair#*=}"

          # archlinux is amd64-only
          if [[ "$os" == "archlinux" && "$cpuv" == "arm64" ]]; then
            continue
          fi

          local container
          container="ghcr.io/gemc/g4install:${g4v}-${os}-${ver}"

          body+="${sep}{"
          body+="\"image\":\"${os}\","
          body+="\"image_tag\":\"${ver}\","
          body+="\"geant4_tag\":\"${g4v}\","
          body+="\"gemc_tag\":\"${gemcv}\","
          body+="\"arch\":\"${cpuv}\","
          body+="\"container\":\"${container}\","
          body+="\"platform\":\"${platform}\","
          body+="\"runner\":\"${runner}\","
          body+="\"suffix\":\"${suffix}\","
          body+="\"logs_dir\":\"${logs_dir}\""
          body+="}"
          sep=","
        done
      done
    done
  done

  print_json "{\"include\":[${body}]}"
}

build_matrix_sanitize() {
  local g4_list arch_list gemc_list
  g4_list="$(get_geant4_tags)"
  arch_list="$(get_cpu_architectures)"
  gemc_list="$(get_gemc_tags)"

  local -a g4_tags arch_tags gemc_tags
  read -r -a g4_tags <<<"$g4_list"
  read -r -a arch_tags <<<"$arch_list"
  read -r -a gemc_tags <<<"$gemc_list"

  local body="" sep="" pair os ver
  for g4v in "${g4_tags[@]}"; do
    for cpuv in "${arch_tags[@]}"; do
      for gemcv in "${gemc_tags[@]}"; do
        for pair in "${OS_VERSIONS[@]}"; do
          os="${pair%%=*}"
          ver="${pair#*=}"

          # archlinux is amd64-only
          if [[ "$os" == "archlinux" && "$cpuv" == "arm64" ]]; then
            continue
          fi

          local container runner sanitizer_list sanitizer_tags
          container="ghcr.io/gemc/g4install:${g4v}-${os}-${ver}"
          runner="$(get_runner "$cpuv")"
          sanitizer_list="$(get_sanitizers "$os")"
          read -r -a sanitizer_tags <<<"$sanitizer_list"

          for sanitizer in "${sanitizer_tags[@]}"; do
            body+="${sep}{"
            body+="\"image\":\"${os}\","
            body+="\"image_tag\":\"${ver}\","
            body+="\"geant4_tag\":\"${g4v}\","
            body+="\"gemc_tag\":\"${gemcv}\","
            body+="\"arch\":\"${cpuv}\","
            body+="\"container\":\"${container}\","
            body+="\"runner\":\"${runner}\","
            body+="\"sanitizer\":\"${sanitizer}\""
            body+="}"
            sep=","
          done
        done
      done
    done
  done

  print_json "{\"include\":[${body}]}"
}

build_matrix_manifest() {
  local g4_list gemc_list
  g4_list="$(get_geant4_tags)"
  gemc_list="$(get_gemc_tags)"

  local -a g4_tags gemc_tags
  read -r -a g4_tags <<<"$g4_list"
  read -r -a gemc_tags <<<"$gemc_list"

  local body="" sep="" pair os ver
  for g4v in "${g4_tags[@]}"; do
    for gemcv in "${gemc_tags[@]}"; do
      for pair in "${OS_VERSIONS[@]}"; do
        os="${pair%%=*}"
        ver="${pair#*=}"

        body+="${sep}{"
        body+="\"image\":\"${os}\","
        body+="\"image_tag\":\"${ver}\","
        body+="\"geant4_tag\":\"${g4v}\","
        body+="\"gemc_tag\":\"${gemcv}\""
        body+="}"
        sep=","
      done
    done
  done

  local json="{\"include\":[${body}]}"
  if command -v jq >/dev/null 2>&1; then
    # De-duplicate by the fields that define one manifest.
    printf '%s' "$json" | jq -c '{
      include: (
        .include
        | unique_by([.geant4_tag,.image,.image_tag,.gemc_tag])
      )
    }'
  else
    printf '%s' "$json"
  fi
}

main() {
  local image_ref
  image_ref="$(build_image_ref)"

  if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
    local delim_build="MATRIX_BUILD_$(date +%s%N)"
    local delim_sanitize="MATRIX_SANITIZE_$(date +%s%N)"
    local delim_manifest="MATRIX_MANIFEST_$(date +%s%N)"
    {
      echo "matrix_build<<$delim_build"
      build_matrix_build
      echo "$delim_build"

      echo "matrix_sanitize<<$delim_sanitize"
      build_matrix_sanitize
      echo "$delim_sanitize"

      echo "matrix_manifest<<$delim_manifest"
      build_matrix_manifest
      echo "$delim_manifest"

      echo "image=$image_ref"
    } >>"$GITHUB_OUTPUT"
  else
    echo "== matrix_build =="
    build_matrix_build
    echo
    echo "== matrix_sanitize =="
    build_matrix_sanitize
    echo
    echo "== matrix_manifest =="
    build_matrix_manifest
    echo
    echo "images located at: $image_ref"
  fi
}

main "$@"
