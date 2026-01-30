#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ci/manifest.sh SUMMARY_FILE IMAGE TAG [DOWNLOADED_LOGS_DIR]
#
# - SUMMARY_FILE: path to markdown summary file
# - IMAGE: e.g. ghcr.io/gemc/src
# - TAG:   e.g. dev-11.4.0-ubuntu-24.04
# - DOWNLOADED_LOGS_DIR (optional): contains logs-${TAG}-*.tgz

summary_file="${1:?missing SUMMARY_FILE}"
image="${2:?missing IMAGE}"
tag="${3:?missing TAG}"
dl_dir="${4:-}"

{
  printf '## `%s:%s`\n' "${image}" "${tag}"
  printf '\n'
  printf 'Includes:\n'
  if docker buildx imagetools inspect "${image}:${tag}-amd64" >/dev/null 2>&1; then
    printf -- '- amd64\n'
  fi
  if docker buildx imagetools inspect "${image}:${tag}-arm64" >/dev/null 2>&1; then
    printf -- '- arm64\n'
  else
    printf -- '- no arm64. Run with --platform=linux/amd64 in arm64 CPUs. On MacOs you may need to use Rosetta for correct terminal emulation.\n'
  fi
  printf '\n'
  printf '### Pull\n'
  printf '```bash\n'
  printf 'docker pull %s:%s\n' "${image}" "${tag}"
  printf '```\n'
  printf '\n'
  printf '### Run in batch mode\n'
  printf '```bash\n'
  printf 'docker run --rm -it %s:%s bash -l\n' "${image}" "${tag}"
  printf '```\n'
  printf '\n'
  printf '### Run with VNC/noVNC\n'
  printf '_VNC → localhost:5900 (password: change-me)_\n'
  printf '_noVNC → http://localhost:6080/vnc.html (password: change-me)_\n'
  printf '```bash\n'
  printf 'docker run --rm -it $VPORTS $VNC_BIND $VNC_PASS $GEO_FLAGS %s:%s\n' "${image}" "${tag}"
  printf '```\n'
  printf '\n'
  printf '## Tests\n'
  printf '\n'
} >> "${summary_file}"

# If no logs directory provided, stop here.
if [[ -z "${dl_dir}" ]]; then
  printf '_No downloaded logs directory provided._\n' >> "${summary_file}"
  exit 0
fi

if [[ ! -d "${dl_dir}" ]]; then
  printf '_Downloaded logs dir does not exist:_ `%s`\n' "${dl_dir}" >> "${summary_file}"
  exit 0
fi

pattern="${dl_dir}/logs-${tag}-"*.tgz

# Collect tarballs safely when none match.
shopt -s nullglob
# shellcheck disable=SC2206 # intentional pathname expansion
tars=( ${pattern} )
shopt -u nullglob

if (( ${#tars[@]} == 0 )); then
  printf '_No logs tarballs found matching:_ `%s`\n' "${pattern}" >> "${summary_file}"
  exit 0
fi

# Sort for stable output (amd64 then arm64, etc.)
IFS=$'\n' sorted=( $(printf '%s\n' "${tars[@]}" | sort) )
unset IFS

printf '_Found %d tarball(s)._\n\n' "${#sorted[@]}" >> "${summary_file}"

for tarball in "${sorted[@]}"; do
  base="$(basename "${tarball}")"
  arch="${base#logs-${tag}-}"
  arch="${arch%.tgz}"

  work="${dl_dir}/extract-${arch}"
  mkdir -p "${work}"

  # Extract only the test log (avoid unpacking everything).
  tar -xzf "${tarball}" -C "${work}" logs/test.log >/dev/null 2>&1 || true
  test_log="${work}/logs/test.log"

  {
    printf '### %s\n' "${arch}"
    if [[ -f "${test_log}" ]]; then
      printf '```text\n'
      ok_fail="$(tail -n 2 "${test_log}")"

      if grep -q '^Summary of Failures:' "${test_log}"; then
        sed -n '/^Summary of Failures:/,/^Ok:/p' "${test_log}" | sed '$d'
        printf '\n'
      else
        printf 'Summary of Failures:\n'
        printf '(none)\n'
        printf '\n'
      fi

      printf '%s\n' "${ok_fail}"
      printf '```\n'
    else
      printf '_No test log found at:_ `%s`\n' "${test_log}"
    fi
    printf '\n'
  } >> "${summary_file}"
done
