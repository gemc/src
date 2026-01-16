#!/usr/bin/env bash
set -euo pipefail

get_geant4_tag() { echo "11.4.0"; } # only one allowed
get_gemc_tags() { echo "dev"; }
get_cpu_architectures() { echo "arm64 amd64"; } # space separated list.
get_runner() {
	local arch=$1
	case "$arch" in
		"arm64") echo "ubuntu-24.04-arm" ;;
		"amd64") echo "ubuntu-latest" ;;
		*)
			echo   "ERROR: unsupported arch $arch" >&2
			return 2
			;;
	esac
}
lc() { printf '%s' "$1" | tr '[:upper:]' '[:lower:]'; } # portable lowercasing

# Single source of truth (order preserved)
OS_VERSIONS=(
  "ubuntu=24.04"
  "fedora=40"
  "almalinux=9.4"
  "debian=12"
  "archlinux=latest"
)

# full sanitizers:
# on macOS, LeakSanitizer (LSan) support is materially less uniform and
# more fragile across toolchains and architectures than ASan/UBSan/TSan.
# macos:  address, thread, undefined
# linux:  address, thread, undefined, leak
#
# In the CI we first run one sanitizer at a time until all issues are solved
# Then we can run them all
get_sanitizers() {
  local choice_of_sanitizer="undefined"
  local baseos=$1
	case "$baseos" in
	  "macos") echo $choice_of_sanitizer ;;
	  "ubuntu" | "fedora" | "almalinux" | "debian" | "archlinux")	echo $choice_of_sanitizer ;;
		*)
			echo   "ERROR: unsupported baseos $baseos" >&2
			return 2
			;;
	esac

}

build_matrix_build() {

	local arch_list gemc_list
	arch_list="$(get_cpu_architectures)"
  gemc_list="$(get_gemc_tags)"

	local -a arch_tags gemc_tags
	read -r -a arch_tags <<<"$arch_list"
	read -r -a gemc_tags <<<"$gemc_list"

  g4v="$(get_geant4_tag)"

	local body="" sep="" pair os ver
	for cpuv in "${arch_tags[@]}"; do
    for gemcv in "${gemc_tags[@]}"; do
      for pair in "${OS_VERSIONS[@]}"; do
        os="${pair%%=*}"
        ver="${pair#*=}"

        local runner="$(get_runner "$cpuv")"

        local sanitizer_list="$(get_sanitizers "$os")"
        local sanitizer_tags
	      read -r -a sanitizer_tags <<<"$sanitizer_list"
	      for sanitizer in "${sanitizer_tags[@]}"; do

          # archlinux is amd64-only
          if [[ "$os" == "archlinux" && "$cpuv" == "arm64" ]]; then
            continue
          fi

          body+="${sep}{"
          body+="\"container\":\"ghcr.io/gemc/g4install:${g4v}-${os}-${ver}\","
          body+="\"baseos\":\"${os}\","
          body+="\"runner\":\"${runner}\","
          body+="\"g4v\":\"${g4v}\","
          body+="\"arch\":\"${cpuv}\","
          body+="\"sanitizer\":\"${sanitizer}\""
          body+="}"
          sep=","
			  done
			done
		done
	done

	local json="{\"include\":[${body}]}"
	if command -v jq >/dev/null 2>&1; then
	#	printf '%s' "$json"
		printf '%s' "$json" | jq -c .
	else
		printf '%s' "$json"
	fi
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

# the separate matrices are needed so that manifest is not run twice
main() {
	local image_ref
	image_ref="$(build_image_ref)"

	if [[ -n "${GITHUB_OUTPUT:-}" ]]; then
		local DELIM_BUILD="MATRIX_SANITIZE_$(date +%s%N)"
		{
			echo "matrix_sanitize<<$DELIM_BUILD"
			build_matrix_build
			echo "$DELIM_BUILD"

			echo "image=$image_ref"
		} >>"$GITHUB_OUTPUT"
	else
		echo "== matrix_sanitize =="
		build_matrix_build
		echo
	fi

}

main "$@"
