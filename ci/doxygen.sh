#!/usr/bin/env zsh
# Purpose: creates the doxygen documentation for the selected classes

set -euo pipefail

# ---- config ----
classes=(
  goptions guts gfields glogging gfactory gtouchable ghit gtranslationTable
  gdata gdynamicDigitization g4display g4dialog
)

# If you still want a second list, keep it, but it's unused by default.
classes_to_do=(
  gdetector gtranslationTable gsplash gui textProgressBar g4system gparticle
  gstreamer userActions gQtButtonsWidget gphysics gsystem utilities
  eventDispenser gsd
)

script_dir="${0:A:h}"

usage() {
  cat <<'EOF'
Usage:
  ./ci/doxygen.sh                 Run doxygen for all default classes
  ./ci/doxygen.sh -c <class>      Run doxygen for a single class (e.g. -c gdata)
  ./ci/doxygen.sh --class <class> Same as -c
  ./ci/doxygen.sh -l              List available classes
  ./ci/doxygen.sh --list          Same as -l
  ./ci/doxygen.sh -h              Show this help
  ./ci/doxygen.sh --help          Same as -h

Notes:
  - Runs from the repo root (recommended).
  - Output is written to ./pages/<class>/ (and then an index via generate_html.py).
EOF
}

list_classes() {
  print -l -- $classes
}

die() {
  print -u2 -- "ERROR: $*"
  exit 2
}

selected_class=""

# ---- args ----
while (( $# )); do
  case "$1" in
    -c|--class)
      shift
      (( $# )) || die "Missing argument for --class"
      selected_class="$1"
      ;;
    -l|--list)
      list_classes
      exit 0
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      break
      ;;
    *)
      die "Unknown option: $1 (use -h for help)"
      ;;
  esac
  shift
done

# ---- preflight ----
echo " "
echo " Doxygen version: $(doxygen --version)"
echo " "

# Determine which classes to build
classes_to_build=($classes)
if [[ -n "$selected_class" ]]; then
  if ! (( ${classes[(I)$selected_class]} )); then
    die "Unknown class '$selected_class'. Use -l to list valid classes."
  fi
  classes_to_build=("$selected_class")
fi

# ---- build ----
rm -rf pages
mkdir -p pages

# Copy stylesheet once into repo root if needed by Doxyfiles; keep behavior but avoid repeated copies.
cp -f doc/mydoxygen.css .

for class in $classes_to_build; do
  echo " Running Doxygen for $class"

  [[ -d "$class" ]] || die "Directory '$class' not found (run from repo root?)"

  pushd "$class" >/dev/null

  # If create_doxygen.sh is needed only to silence warnings, keep it but don't fail the build if it isn't present.
  if [[ -x "$script_dir/create_doxygen.sh" ]]; then
    "$script_dir/create_doxygen.sh" "$class"
  fi

  [[ -f "Doxyfile" ]] || die "Missing Doxyfile in '$class/'"

  doxygen Doxyfile

  mkdir -p "../pages/$class"

  # Move generated HTML if present
  if [[ -d "html" ]]; then
    mv html/* "../pages/$class/"
    rm -rf html
  else
    die "No 'html/' output produced for $class"
  fi

  popd >/dev/null
  echo
done

# Build top-level index / landing page
if [[ -f "$script_dir/generate_html.py" ]]; then
  "$script_dir/generate_html.py"
else
  # preserve your original call path too
  ./ci/generate_html.py
fi
