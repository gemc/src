#!/usr/bin/env zsh
# Purpose: build doxygen docs per module with cross-references (2-pass: TAGs then HTML)

set -euo pipefail

# ---- config ----
doc_modules=(
  goptions guts gfields glogging gfactory gtouchable ghit gtranslationTable
  gdata gdynamicDigitization g4display g4dialog
)

script_dir="${0:A:h}"
pages_dir="pages"



usage() {
  cat <<'EOF'
Usage:
  ./ci/doxygen.sh                 Build all default modules
  ./ci/doxygen.sh -c <module>     Build a single module (e.g. -c gdata)
  ./ci/doxygen.sh -l              List available modules
  ./ci/doxygen.sh -h              Help

Notes:
  - Output is written to ./pages/<module>/
  - Two-pass build:
    1) TAG-only pass for all selected modules
    2) HTML pass for all selected modules with TAGFILES cross-links
  - If you build a single module (-c), existing ./pages is preserved for cross-links.
EOF
}

list_modules() { print -l -- $doc_modules; }

die() { print -u2 -- "ERROR: $*"; exit 2; }

# Portable in-place sed (BSD/macOS vs GNU)
inplace_sed() {
  local expr="$1"
  local file="$2"
  if sed --version >/dev/null 2>&1; then
    sed -i "$expr" "$file"
  else
    sed -i '' "$expr" "$file"
  fi
}

# Find module directory: supports either ./<module>/ or ./modules/<module>/
module_dir_for() {
  local m="$1"
  if [[ -d "modules/$m" ]]; then
    print -- "modules/$m"
  elif [[ -d "$m" ]]; then
    print -- "$m"
  else
    return 1
  fi
}

selected=""
while (( $# )); do
  case "$1" in
    -c|--class|--module)
      shift; (( $# )) || die "Missing argument for -c/--module"
      selected="$1"
      ;;
    -l|--list) list_modules; exit 0 ;;
    -h|--help) usage; exit 0 ;;
    --) shift; break ;;
    *) die "Unknown option: $1 (use -h for help)" ;;
  esac
  shift
done

echo ""
echo " Doxygen version: $(doxygen --version)"
echo ""

modules_to_build=($doc_modules)
if [[ -n "$selected" ]]; then
  if ! (( ${doc_modules[(I)$selected]} )); then
    die "Unknown module '$selected'. Use -l to list valid modules."
  fi
  modules_to_build=("$selected")
fi

# ---- pages handling ----
if [[ -z "$selected" ]]; then
  rm -rf "$pages_dir"
  mkdir -p "$pages_dir"
else
  rm -rf "$pages_dir/$selected"
  mkdir -p "$pages_dir/$selected"
fi

# Copy CSS into each module output after HTML pass; keep source of truth in doc/mydoxygen.css
css_src="doc/mydoxygen.css"
[[ -f "$css_src" ]] || css_src="../doc/mydoxygen.css"

# Absolute path to the canonical stylesheet in the repo root
repo_root="${script_dir:h}"
css_abs="$repo_root/doc/mydoxygen.css"
[[ -f "$css_abs" ]] || die "Missing stylesheet: $css_abs"

# ---- PASS 1: TAG-only ----
for m in $modules_to_build; do
  echo " [PASS 1] Generating TAG for $m"
  moddir="$(module_dir_for "$m")" || die "Module dir not found for '$m'"
  mkdir -p "$pages_dir/$m"

  pushd "$moddir" >/dev/null
  ln -sf "$css_abs" mydoxygen.css

  "$script_dir/create_doxygen.sh" "$m"
  [[ -f Doxyfile ]] || die "Missing Doxyfile in '$moddir'"

  inplace_sed 's|^GENERATE_HTML[[:space:]]*=.*|GENERATE_HTML              = NO|g' Doxyfile
  inplace_sed '/^TAGFILES\b/d' Doxyfile

  doxygen Doxyfile
  [[ -f "../$pages_dir/$m/$m.tag" ]] || die "Tag file not produced: $pages_dir/$m/$m.tag"

  popd >/dev/null
  echo
done

# ---- PASS 2: HTML + TAGFILES ----
for m in $modules_to_build; do
  echo " [PASS 2] Generating HTML for $m (with cross-links)"
  moddir="$(module_dir_for "$m")" || die "Module dir not found for '$m'"
  mkdir -p "$pages_dir/$m"

  pushd "$moddir" >/dev/null
  ln -sf "$css_abs" mydoxygen.css

  "$script_dir/create_doxygen.sh" "$m"
  [[ -f Doxyfile ]] || die "Missing Doxyfile in '$moddir'"

  inplace_sed 's|^GENERATE_HTML[[:space:]]*=.*|GENERATE_HTML              = YES|g' Doxyfile
  inplace_sed '/^TAGFILES\b/d' Doxyfile

  for other in $doc_modules; do
    [[ "$other" == "$m" ]] && continue
    if [[ -f "../$pages_dir/$other/$other.tag" ]]; then
      echo "TAGFILES += ../$pages_dir/$other/$other.tag=../$other" >> Doxyfile
    fi
  done

  doxygen Doxyfile

  popd >/dev/null

  cp -f "$css_src" "$pages_dir/$m/mydoxygen.css"  2>/dev/null || true
  [[ -f "$pages_dir/$m/index.html" ]] || die "HTML not produced for $m ($pages_dir/$m/index.html missing)"
  echo
done

# Build top-level index / landing page
if [[ -f "$script_dir/generate_html.py" ]]; then
  "$script_dir/generate_html.py"
elif [[ -f "./ci/generate_html.py" ]]; then
  ./ci/generate_html.py
fi

echo "Done. Output in ./$pages_dir/"
