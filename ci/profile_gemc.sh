#!/usr/bin/env zsh

# Purpose: profile one installed GEMC example under Valgrind's callgrind tool.
#
# Container run:
# docker run --rm -it ghcr.io/gemc/g4install:11.4.2-ubuntu-24.04 bash -li
# git clone http://github.com/gemc/src /root/src && cd /root/src
# ./ci/build.sh profile
# apt-get update && apt-get install -y valgrind
# ./ci/profile_gemc.sh -e optical/cherenkov -n 2000
#
# The callgrind output file is meant to be opened with qcachegrind / kcachegrind;
# see the Valgrind Profile workflow summary for a reading guide.

# Capture the repository root before sourcing env.sh: the run below cds into a scratch
# working directory, so all output paths are resolved as absolute up front.
repo_root="$PWD"

# Load environment variables: install location ($GEMC), log paths, helper functions.
source ci/env.sh

nevents=2000
example=""

Help() {
	echo
	echo "Syntax: profile_gemc.sh [-h|e|n]"
	echo
	echo "Options:"
	echo
	echo "-h: Print this Help."
	echo "-e: example to profile, as <branch>/<name> (e.g. optical/cherenkov)."
	echo "-n: number of events to profile (default: $nevents)."
	echo
	echo "Supported examples: basic/scintillator_barrel, fields/torus, optical/cherenkov,"
	echo "                    optical/mirrors, advanced/sro."
	echo
}

if [ $# -eq 0 ]; then
	Help
	exit 1
fi

while getopts ":he:n:" option; do
	case $option in
		h)
			Help
			exit
			;;
		e)
			example="$OPTARG"
			;;
		n)
			nevents="$OPTARG"
			;;
		\?)
			echo "Error: Invalid option"
			exit 1
			;;
	esac
done

if [[ -z $example ]]; then
	echo "Error: an example is required (-e <branch>/<name>)."
	Help
	exit 1
fi

install_dir="$GEMC/dev"
gemc_bin="$install_dir/bin/gemc"
python_bin="$install_dir/python_env/bin/python3"
example_name="${example:t}"
source_dir="$install_dir/examples/$example"

if [[ ! -x $gemc_bin ]]; then
	echo "Error: gemc executable not found at $gemc_bin. Build with './ci/build.sh profile' first."
	exit 1
fi
if [[ ! -d $source_dir ]]; then
	echo "Error: example directory not found at $source_dir."
	exit 1
fi

# Per-example gemc arguments. Every example is driven from freshly generated ascii geometry
# (see below), so the gsystem factory is overridden to ascii regardless of what the installed
# YAML card selects. torus ships as a field-display-only card with no beam, so a muon beam is
# injected here to make its 2000 events actually traverse the field. sro already selects the
# ascii factory and the sro streamer in its YAML, and its plugin is found on the install lib path.
case $example in
	basic/scintillator_barrel)
		extra_args=('-gsystem=[{name: scintillator_barrel, factory: ascii}]')
		;;
	optical/cherenkov)
		extra_args=('-gsystem=[{name: cherenkov, factory: ascii}]')
		;;
	optical/mirrors)
		extra_args=('-gsystem=[{name: mirrors, factory: ascii}]')
		;;
	fields/torus)
		extra_args=(
			'-gsystem=[{name: torus, factory: ascii}]'
			'-gparticle=[{name: mu-, p: 2*GeV, theta: 20*deg, delta_phi: 360*deg, multiplicity: 6}]'
		)
		;;
	advanced/sro)
		extra_args=()
		;;
	*)
		echo "Error: unknown example '$example'."
		Help
		exit 1
		;;
esac

# Work in a scratch copy of the installed example so generated geometry and stream output do
# not pollute the install tree. Field maps and other assets travel with the copy.
workdir=$(mktemp -d "${TMPDIR:-/tmp}/gemc-profile-${example_name}.XXXXXX")
trap 'rm -rf "$workdir"' EXIT
cp -a "$source_dir/." "$workdir/"
cd "$workdir" || exit 1

echo " > Generating ascii geometry for $example"
if ! "$python_bin" "$workdir/$example_name.py" -f ascii; then
	echo "Error: geometry generation failed for $example."
	exit 1
fi

output_dir="${PROFILE_OUTPUT_DIR:-$repo_root/profile-logs}"
mkdir -p "$output_dir"
example_safe="${example//\//_}"
callgrind_out="$output_dir/callgrind.out.$example_safe"
gemc_log="$output_dir/gemc.$example_safe.log"

echo " > Profiling $example with $nevents events under callgrind (single thread)"
echo "   $gemc_bin $example_name.yaml ${extra_args[*]} -n=$nevents -nthreads=1"
# Cache and branch simulation add the miss counts the CEst (cycle-estimation) formula needs, so
# qcachegrind and ci/profile_summary.py can report estimated cycles instead of raw instruction reads.
valgrind --tool=callgrind \
	--callgrind-out-file="$callgrind_out" \
	--dump-instr=yes --collect-jumps=yes --skip-plt=yes \
	--cache-sim=yes --branch-sim=yes \
	"$gemc_bin" "$example_name.yaml" \
	"${extra_args[@]}" \
	-n="$nevents" -nthreads=1 | tee "$gemc_log"
exitCode=${pipestatus[1]}

if [[ $exitCode -ne 0 ]]; then
	echo " > gemc exited with code $exitCode"
	exit $exitCode
fi

echo
echo " > callgrind profile written to $callgrind_out"
echo " > open it with:  qcachegrind $callgrind_out"

# Per-category CEst table (best effort: needs callgrind_annotate from the valgrind package).
summary_md="$output_dir/summary.$example_safe.md"
if command -v callgrind_annotate >/dev/null 2>&1; then
	echo " > Writing category summary to $summary_md"
	python3 "$repo_root/ci/profile_summary.py" "$callgrind_out" --title "$example" > "$summary_md" || \
		echo " > Category summary generation failed; the callgrind file is still available."
else
	echo " > callgrind_annotate not found; skipping the category summary."
fi

ls -l "$output_dir"
echo
