#!/usr/bin/env bash

# Run the standard local GEMC scaling benchmark and retain a machine-specific summary.
set -euo pipefail

repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
threadscale_ref="${THREADSCALE_REF:-main}"
workload=20000
cores_workload_scale=0
max_threads=""
output_path="thread-scaling"
gemc_options=""
output_mode="compare-root-output"
output_mode_selected=false
threadscale_options=()

usage() {
  printf '%s\n' \
    "Usage: bin/scaling.sh [OPTIONS] [-- TEST_SCALING_OPTIONS]" \
    "" \
    "Options:" \
    "  --workload N        events per GEMC invocation (default: 20000)" \
    "  --cores-workload-scale F" \
    "                      add F times the one-thread workload per additional thread (default: 0)" \
    "  --max-threads N     cap detected CPUs; omit to use every visible CPU" \
    "  --output-dir DIR    report directory (default: thread-scaling)" \
    "  --threadscale-ref R ThreadScale branch or tag (default: main)" \
    "  --gemc-options OPTS additional GEMC arguments applied to every run" \
    "  --compare-root-output  compare no output with ROOT output (default)" \
    "  --without-output       disable all output streamers" \
    "  --with-output          keep the YAML-configured output streamers" \
    "  --with-root-output     replace configured streamers with ROOT output only" \
    "  -h, --help          show this help" \
    "" \
    "Options after -- are passed to test_scaling after the defaults." \
    "" \
    "Workload scaling:" \
    "  W(N) = W(1) * [1 + (N - 1) * F]" \
    "  To target similar runtimes, first measure fixed-work throughput speedup S(N), then use" \
    "  F = [S(N) - 1] / (N - 1). Example: S(64) = 6.65 gives F = 0.09; choosing" \
    "  F = 0.1 scales 20000 events to 146000 events at 64 threads."
}

while (( $# > 0 )); do
  case "$1" in
    --workload | --cores-workload-scale | --max-threads | --output-dir | --threadscale-ref | --gemc-options)
      if (( $# < 2 )); then
        echo "$1 requires a value" >&2
        exit 2
      fi
      option="$1"
      value="$2"
      shift 2
      case "${option}" in
        --workload) workload="${value}" ;;
        --cores-workload-scale) cores_workload_scale="${value}" ;;
        --max-threads) max_threads="${value}" ;;
        --output-dir) output_path="${value}" ;;
        --threadscale-ref) threadscale_ref="${value}" ;;
        --gemc-options) gemc_options="${value}" ;;
      esac
      ;;
    --compare-root-output | --without-output | --with-output | --with-root-output)
      if [[ "${output_mode_selected}" == true ]]; then
        echo "Select only one output mode" >&2
        exit 2
      fi
      output_mode="${1#--}"
      output_mode_selected=true
      shift
      ;;
    -h | --help)
      usage
      exit 0
      ;;
    --)
      shift
      threadscale_options=("$@")
      break
      ;;
    *)
      echo "Unknown option: $1" >&2
      echo "Place test_scaling options after --." >&2
      exit 2
      ;;
  esac
done

if [[ ! "${workload}" =~ ^[1-9][0-9]*$ ]]; then
  echo "--workload must be a positive integer: ${workload}" >&2
  exit 2
fi
if [[ ! "${cores_workload_scale}" =~ ^[0-9]+([.][0-9]+)?$ ]]; then
  echo "--cores-workload-scale must be a non-negative number: ${cores_workload_scale}" >&2
  exit 2
fi
if [[ -n "${max_threads}" && ! "${max_threads}" =~ ^[0-9]+$ ]]; then
  echo "--max-threads must be a non-negative integer: ${max_threads}" >&2
  exit 2
fi
if [[ -z "${output_path}" ]]; then
  echo "--output-dir must not be empty" >&2
  exit 2
fi
for option in "${threadscale_options[@]}"; do
  case "${option}" in
    --benchmarks | --workload | --cores-workload-scale | --max-threads | --output-dir)
      echo "Pass ${option} before -- so the wrapper uses the same value." >&2
      exit 2
      ;;
  esac
done

if [[ "${output_path}" == /* ]]; then
  output_directory="${output_path}"
else
  output_directory="${repository_root}/${output_path}"
fi
partial_directory="${output_directory}.parts"

for required_command in gemc git node python3; do
  if ! command -v "${required_command}" >/dev/null 2>&1; then
    echo "Required command not found in PATH: ${required_command}" >&2
    exit 1
  fi
done

if [[ -e "${output_directory}" || -e "${partial_directory}" ]]; then
  echo "Remove or rename existing thread-scaling output before running this script:" >&2
  echo "  ${output_directory}" >&2
  echo "  ${partial_directory}" >&2
  exit 1
fi

threadscale_tmp="$(mktemp -d "${TMPDIR:-/tmp}/gemc-threadscale.XXXXXX")"
cleanup() {
  rm -rf -- "${threadscale_tmp}"
}
trap cleanup EXIT

git clone --quiet --depth 1 --branch "${threadscale_ref}" \
  https://github.com/gemc/ThreadScale.git "${threadscale_tmp}/ThreadScale"

cd "${repository_root}"
gemc_command='gemc examples/basic/scintillator_barrel/scintillator_barrel.yaml'
gemc_command+=' -n={workload} -nthreads={threads}'
if [[ -n "${gemc_options}" ]]; then
  gemc_command+=" ${gemc_options}"
fi
test_scaling_arguments=()
case "${output_mode}" in
  compare-root-output)
    benchmarks_file="${threadscale_tmp}/benchmarks.json"
    python3 - "${benchmarks_file}" "${gemc_command}" "${workload}" <<'PY'
import json
import sys

output_file, command, workload = sys.argv[1], sys.argv[2], int(sys.argv[3])
common = {
    "working_directory": ".",
    "workload": workload,
    "workload_unit": "events",
    "comparison_group": "scintillator-barrel output comparison",
}
benchmarks = [
    {
        **common,
        "name": "scintillator-barrel: no output",
        "command": f"{command} -gstreamer=[]",
        "comparison_label": "No output",
    },
    {
        **common,
        "name": "scintillator-barrel: ROOT output",
        "command": f"{command} '-gstreamer=[{{format: root, filename: barrel}}]'",
        "comparison_label": "ROOT output",
    },
]
with open(output_file, "w", encoding="utf-8") as stream:
    json.dump(benchmarks, stream, indent=2)
    stream.write("\n")
PY
    test_scaling_arguments+=(--benchmarks "${benchmarks_file}")
    ;;
  without-output)
    test_scaling_arguments+=("${gemc_command} -gstreamer=[]" --name scintillator-barrel)
    ;;
  with-output)
    test_scaling_arguments+=("${gemc_command}" --name scintillator-barrel)
    ;;
  with-root-output)
    gemc_command+=" '-gstreamer=[{format: root, filename: barrel}]'"
    test_scaling_arguments+=("${gemc_command}" --name scintillator-barrel)
    ;;
esac
test_scaling_arguments+=(
  --threads powers-of-two
  --duration 0
  --runs 4
  --warmup-runs 1
  --summary-plots rate
  --output-dir "${output_directory}"
)
if [[ "${output_mode}" != "compare-root-output" ]]; then
  test_scaling_arguments+=(--workload "${workload}" --workload-unit events)
fi
if [[ "${cores_workload_scale}" != "0" ]]; then
  test_scaling_arguments+=(--cores-workload-scale "${cores_workload_scale}")
fi
if [[ -n "${max_threads}" ]]; then
  test_scaling_arguments+=(--max-threads "${max_threads}")
fi
test_scaling_arguments+=("${threadscale_options[@]}")

"${threadscale_tmp}/ThreadScale/test_scaling" "${test_scaling_arguments[@]}"
summary_file="$(python3 ci/name_thread_scaling_summary.py "${output_directory}")"
echo "Wrote ${summary_file}"
