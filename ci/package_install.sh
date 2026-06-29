#!/usr/bin/env bash
set -euo pipefail

# Build a relocatable GEMC install tarball from an existing Meson install prefix.
#
# Usage:
#   ci/package_install.sh [INSTALL_PREFIX] [OUTPUT_DIR] [PACKAGE_NAME]
#
# The package intentionally excludes python_env. pygemc is distributed through
# pip, while this archive is for the GEMC executable and installed support files.

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "${script_dir}/.." && pwd)"

if [[ $# -gt 3 ]]; then
  echo "Usage: $0 [INSTALL_PREFIX] [OUTPUT_DIR] [PACKAGE_NAME]" >&2
  exit 2
fi

install_prefix="${1:-${SIM_HOME:?SIM_HOME not set}/gemc/dev}"
output_dir="${2:-${repo_root}/dist}"
geant4_version="${GEANT4_VERSION:-11.4.1}"
arch="$(uname -m)"
case "${arch}" in
  x86_64) arch=amd64 ;;
  aarch64|arm64) arch=arm64 ;;
esac

gemc_version="${GEMC_PACKAGE_VERSION:-$("${script_dir}/gemc_version.sh")}"
package_name="${3:-gemc-${gemc_version}-geant4-${geant4_version}-linux-${arch}}"

if [[ ! -d "${install_prefix}" ]]; then
  echo "Install prefix does not exist: ${install_prefix}" >&2
  exit 1
fi

mkdir -p "${output_dir}"
stage="$(mktemp -d)"
trap 'rm -rf "${stage}"' EXIT

package_root="${stage}/${package_name}"
cp -a "${install_prefix}" "${package_root}"

# pygemc can be installed with pip; do not ship the installed virtualenv.
rm -rf "${package_root}/python_env"

# Keep the installed bin directory small and predictable.
keep_bins=(
  gemc
  test_gdata_event_verbose
  test_ghit
  test_gstreamer_csv
  test_gdynamic_plugin_load
  test_event_dispenser
)

is_kept_bin() {
  local name="$1"
  local keep
  for keep in "${keep_bins[@]}"; do
    [[ "${name}" == "${keep}" ]] && return 0
  done
  return 1
}

if [[ -d "${package_root}/bin" ]]; then
  while IFS= read -r -d '' entry; do
    name="$(basename "${entry}")"
    if ! is_kept_bin "${name}"; then
      rm -f "${entry}"
    fi
  done < <(find "${package_root}/bin" -mindepth 1 -maxdepth 1 \( -type f -o -type l \) -print0)
fi

archive_name_from_data_dir() {
  local directory="$1"
  local prefix version

  if [[ "${directory}" =~ ^(G4)?([A-Za-z]+)([0-9].*)$ ]]; then
    prefix="${BASH_REMATCH[1]}${BASH_REMATCH[2]}"
    version="${BASH_REMATCH[3]}"
  else
    echo "Cannot derive Geant4 dataset archive name from directory: ${directory}" >&2
    return 1
  fi

  if [[ "${prefix}" == G4* ]]; then
    printf '%s.%s.tar.gz\n' "${prefix}" "${version}"
  else
    printf 'G4%s.%s.tar.gz\n' "${prefix}" "${version}"
  fi
}

geant4_dataset_records=()

# Populate the G4*DATA descriptors from geant4-config only when they are not
# already exported. A relocated geant4.env (the macOS tarball flow) exports them
# pointing at the data it actually installed; geant4-config instead reports the
# absolute paths baked in at build time, which do not exist after relocation.
# Do not use `env | grep -q` here: with pipefail, grep's early exit can give env
# SIGPIPE and make a successful match look like a failed pipeline.
geant4_data_env_found=false
for env_name in "${!G4@}"; do
  if [[ "${env_name}" =~ ^G4[A-Z0-9_]*DATA$ && -n "${!env_name:-}" ]]; then
    geant4_data_env_found=true
    break
  fi
done

if [[ "${geant4_data_env_found}" == false ]]; then
  if command -v geant4-config >/dev/null 2>&1; then
    eval "$(geant4-config --sh)"
  fi
fi

while IFS='=' read -r env_name env_path; do
  [[ -n "${env_name}" && -n "${env_path}" ]] || continue
  # The dataset records only need the dataset directory NAME (basename): it is what drives the gemc
  # tarball's own gemc.env / install_geant4_data.sh, which download the data fresh on the end-user
  # machine. So a G4*DATA pointing at a directory that does not exist here is not fatal. A relocated
  # Geant4 tarball (the macOS flow) exports G4*DATA pointing at the original build tree, which is
  # gone after relocation; prefer the actual relocated data dir under GEANT4_DATA_DIR when present,
  # otherwise just warn and keep the (still-correct) dataset name.
  data_dir_name="$(basename "${env_path}")"
  if [[ ! -d "${env_path}" ]]; then
    relocated="${GEANT4_DATA_DIR:+${GEANT4_DATA_DIR}/${data_dir_name}}"
    if [[ -n "${relocated}" && -d "${relocated}" ]]; then
      env_path="${relocated}"
    else
      echo "Warning: Geant4 data ${env_name} directory not found (${env_path}); using dataset name" \
           "'${data_dir_name}' for the package descriptors." >&2
    fi
  fi
  archive_name="$(archive_name_from_data_dir "${data_dir_name}")"
  geant4_dataset_records+=( "${env_name}|${archive_name}|${data_dir_name}" )
done < <(env | LC_ALL=C sort | grep -E '^G4[A-Z0-9_]*DATA=' || true)

if (( ${#geant4_dataset_records[@]} == 0 )); then
  echo "No Geant4 data environment variables were found." >&2
  echo "Load the Geant4 environment before running $0." >&2
  exit 1
fi

cat > "${package_root}/gemc.env" <<'EOF'
# Source this file after unpacking the GEMC tarball.
#
# Geant4 data directories are expected under ${GEMC_HOME}/geant4-data.
# Run ${GEMC_HOME}/install_geant4_data.sh to download and unpack them automatically.

if [[ -n "${BASH_SOURCE[0]:-}" ]]; then
  GEMC_ENV_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
elif [[ -n "${ZSH_VERSION:-}" ]]; then
  GEMC_ENV_DIR="$(cd "$(dirname "${(%):-%x}")" && pwd)"
else
  GEMC_ENV_DIR="$(pwd)"
fi

export GEMC_HOME="${GEMC_HOME:-${GEMC_ENV_DIR}}"
export PATH="${GEMC_HOME}/bin:${PATH}"
if [ -d "${GEMC_HOME}/lib" ]; then
  export LD_LIBRARY_PATH="${GEMC_HOME}/lib:${LD_LIBRARY_PATH:-}"
fi

export GEMC_GEANT4_DATA_DIR="${GEMC_HOME}/geant4-data"

gemc_geant4_datasets=(
EOF

for record in "${geant4_dataset_records[@]}"; do
  env_name="${record%%|*}"
  rest="${record#*|}"
  data_dir_name="${rest#*|}"
  printf '  "%s|%s"\n' "${env_name}" "${data_dir_name}" >> "${package_root}/gemc.env"
done

cat >> "${package_root}/gemc.env" <<'EOF'
)

for gemc_dataset in "${gemc_geant4_datasets[@]}"; do
  gemc_env_name="${gemc_dataset%%|*}"
  gemc_data_dir="${gemc_dataset#*|}"
  export "${gemc_env_name}=${GEMC_GEANT4_DATA_DIR}/${gemc_data_dir}"
done

gemc_missing_data=()
for gemc_dataset in "${gemc_geant4_datasets[@]}"; do
  gemc_env_name="${gemc_dataset%%|*}"
  gemc_data_dir="${gemc_dataset#*|}"
  gemc_data_path="${GEMC_GEANT4_DATA_DIR}/${gemc_data_dir}"
  if [[ ! -d "${gemc_data_path}" ]]; then
    gemc_missing_data+=("${gemc_env_name}: ${gemc_data_path}")
  fi
done

if (( ${#gemc_missing_data[@]} > 0 )); then
  echo "GEMC Geant4 data check failed. Missing required data directories:" >&2
  printf '  %s\n' "${gemc_missing_data[@]}" >&2
  echo "Run: ${GEMC_HOME}/install_geant4_data.sh" >&2
  return 1 2>/dev/null || exit 1
fi

unset gemc_data_dir gemc_data_path gemc_dataset gemc_env_name gemc_geant4_datasets gemc_missing_data
EOF

cat > "${package_root}/install_geant4_data.sh" <<'EOF'
#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
data_dir="${script_dir}/geant4-data"
base_url="${GEANT4_DATA_BASE_URL:-https://cern.ch/geant4-data/datasets}"

datasets=(
EOF

for record in "${geant4_dataset_records[@]}"; do
  env_name="${record%%|*}"
  rest="${record#*|}"
  archive_name="${rest%%|*}"
  data_dir_name="${rest#*|}"
  printf '  "%s|%s|%s"\n' "${env_name}" "${archive_name}" "${data_dir_name}" \
    >> "${package_root}/install_geant4_data.sh"
done

cat >> "${package_root}/install_geant4_data.sh" <<'EOF'
)

download() {
  local url="$1"
  local output="$2"
  if command -v curl >/dev/null 2>&1; then
    curl -fL --retry 3 -o "${output}" "${url}"
  elif command -v wget >/dev/null 2>&1; then
    wget -O "${output}" "${url}"
  else
    echo "Neither curl nor wget is available." >&2
    return 1
  fi
}

mkdir -p "${data_dir}"

for item in "${datasets[@]}"; do
  env_name="${item%%|*}"
  rest="${item#*|}"
  archive="${rest%%|*}"
  directory="${rest#*|}"
  target="${data_dir}/${directory}"

  if [[ -d "${target}" ]]; then
    echo "Found ${env_name}: ${directory}"
    continue
  fi

  tmp="$(mktemp -d)"
  trap 'rm -rf "${tmp}"' EXIT
  echo "Downloading ${env_name}: ${directory}"
  download "${base_url}/${archive}" "${tmp}/${archive}"
  tar -xzf "${tmp}/${archive}" -C "${data_dir}"
  rm -rf "${tmp}"
  trap - EXIT

  if [[ ! -d "${target}" ]]; then
    echo "Expected directory was not created: ${target}" >&2
    exit 1
  fi
done

echo "Geant4 data installed in ${data_dir}"
EOF
chmod +x "${package_root}/install_geant4_data.sh"

cat > "${package_root}/INSTALL_TARBALL.md" <<EOF
# GEMC tarball installation

This archive contains the GEMC install tree without \`python_env\`. Install pygemc with pip when you need the
Python API:

\`\`\`bash
python3 -m pip install pygemc
\`\`\`

## Geant4 data

GEMC is statically linked against Geant4, so the Geant4 shared libraries are not included.
Geant4 still loads physics data at runtime. Run:

\`\`\`bash
./install_geant4_data.sh
source ./gemc.env
gemc -v
\`\`\`
EOF

tarball="${output_dir}/${package_name}.tar.gz"
tar -C "${stage}" -czf "${tarball}" "${package_name}"
echo "${tarball}"
