#!/usr/bin/env bash
set -euo pipefail

# Usage:
#   ci/summary.sh [SUMMARY_FILE]
# If SUMMARY_FILE is omitted, uses $GITHUB_STEP_SUMMARY.

summary_file="${1:-${GITHUB_STEP_SUMMARY:-}}"
if [[ -z "${summary_file}" ]]; then
  printf 'ERROR: summary file not provided and GITHUB_STEP_SUMMARY is empty\n' >&2
  exit 2
fi

{
  printf '# GEMC Images — Overview\n'
  printf 'Repository: [gemc images](https://github.com/gemc/src/pkgs/container/src)\n'
  printf 'Except for archlinux (amd64-only), all images are built for both the *amd64* and *arm64* architectures.\n'
  printf '\n'
  printf '\n'
  printf 'Set these convenience variables for the interactive use below (choose your own password)\n'
  printf '```bash\n'
  printf 'VPORTS=(-p 6080:6080 -p 5900:5900)\n'
  printf 'VNC_PASS=(-e X11VNC_PASSWORD=change-me)\n'
  printf 'VNC_BIND=(-e VNC_BIND=0.0.0.0)\n'
  printf 'GEO_FLAGS=(-e GEOMETRY=1920x1200)\n'
  printf '```\n'
  printf '\n'
  printf 'If your arch is arm64 and want to use the archlinux image, add this to all command lines:\n'
  printf '```bash\n'
  printf -- '--platform=linux/amd64\n'
  printf '```\n'
  printf -- '---\n'
} >> "${summary_file}"
