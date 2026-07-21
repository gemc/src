#!/usr/bin/env zsh
# File: ci/doxygen.sh
#
# Purpose: build doxygen docs per module with cross-references (2-pass: TAGs then HTML)

set -euo pipefail

# ---- config ----
doc_modules=(
guts goptions glogging gbase gfactory textProgressBar gtouchable ghit gtranslationTable gdata gboard
gdynamicDigitization
gsystem gstreamer eventDispenser gqtbuttonswidget g4display g4dialog g4system gparticle gphysics gsplash gsd
gfields gdetector dbselect gtree gui actions utilities)

pages_dir="pages"
script_dir="${0:A:h}"
repo_root="${script_dir:h}"
cd "$repo_root"

"$script_dir/generate_doxygen_diagrams.py" --check
"$script_dir/generate_doxygen_concept_diagrams.py" --check

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

# Find module directory: supports current ./gemc/<module>/ layout, plus older
# ./<module>/ and ./modules/<module>/ layouts.
module_dir_for() {
  local m="$1"
  if [[ -d "gemc/$m" ]]; then
    print -- "gemc/$m"
  elif [[ -d "modules/$m" ]]; then
    print -- "modules/$m"
  elif [[ -d "$m" ]]; then
    print -- "$m"
  else
    return 1
  fi
}

pages_rel_for() {
  local moddir="$1"
  if [[ "$moddir" == */* ]]; then
    print -- "../../$pages_dir"
  else
    print -- "../$pages_dir"
  fi
}

set_doxy_key() {
  local key="$1"
  local value="$2"
  local file="$3"
  if grep -qE "^${key}[[:space:]]*=" "$file"; then
    inplace_sed "s|^${key}[[:space:]]*=.*|${key}              = ${value}|g" "$file"
  else
    print -- "${key}              = ${value}" >> "$file"
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
css_src="ci/mydoxygen.css"

css_abs="$repo_root/$css_src"
[[ -f "$css_abs" ]] || die "Missing stylesheet: $css_abs"

# ---- PASS 1: TAG-only ----
for m in $modules_to_build; do
  echo " [PASS 1] Generating TAG for $m"
  moddir="$(module_dir_for "$m")" || die "Module dir not found for '$m'"
  mkdir -p "$pages_dir/$m"
  pages_rel="$(pages_rel_for "$moddir")"

  pushd "$moddir" >/dev/null
  ln -sf "$css_abs" mydoxygen.css

  "$script_dir/create_doxygen.sh" "$m"
  [[ -f Doxyfile ]] || die "Missing Doxyfile in '$moddir'"
  set_doxy_key "OUTPUT_DIRECTORY" "$pages_rel" Doxyfile
  set_doxy_key "GENERATE_TAGFILE" "$pages_rel/$m/$m.tag" Doxyfile

  # Ensure TAG pass doesn't emit HTML
  if grep -qE '^GENERATE_HTML[[:space:]]*=' Doxyfile; then
    inplace_sed 's|^GENERATE_HTML[[:space:]]*=.*|GENERATE_HTML              = NO|g' Doxyfile
  else
    print -- 'GENERATE_HTML              = NO' >> Doxyfile
  fi

  # The tag-only pass cannot resolve links to modules whose tags do not exist yet.
  # Validate documentation during the cross-linked HTML pass instead.
  set_doxy_key "WARNINGS" "NO" Doxyfile
  set_doxy_key "WARN_IF_DOC_ERROR" "NO" Doxyfile
  set_doxy_key "WARN_IF_INCOMPLETE_DOC" "NO" Doxyfile

  # Ensure TAGFILES isn't present in pass 1
  inplace_sed '/^TAGFILES\b/d' Doxyfile

  doxygen Doxyfile
  [[ -f "$pages_rel/$m/$m.tag" ]] || die "Tag file not produced: $pages_dir/$m/$m.tag"

  popd >/dev/null
  echo
done

# ---- PASS 2: HTML + TAGFILES ----
for m in $modules_to_build; do
  echo " [PASS 2] Generating HTML for $m (with cross-links)"
  moddir="$(module_dir_for "$m")" || die "Module dir not found for '$m'"
  mkdir -p "$pages_dir/$m"
  pages_rel="$(pages_rel_for "$moddir")"

  pushd "$moddir" >/dev/null
  ln -sf "$css_abs" mydoxygen.css

  "$script_dir/create_doxygen.sh" "$m"
  [[ -f Doxyfile ]] || die "Missing Doxyfile in '$moddir'"
  set_doxy_key "OUTPUT_DIRECTORY" "$pages_rel" Doxyfile
  set_doxy_key "GENERATE_TAGFILE" "$pages_rel/$m/$m.tag" Doxyfile

  # Ensure HTML is enabled in pass 2
  if grep -qE '^GENERATE_HTML[[:space:]]*=' Doxyfile; then
    inplace_sed 's|^GENERATE_HTML[[:space:]]*=.*|GENERATE_HTML              = YES|g' Doxyfile
  else
    print -- 'GENERATE_HTML              = YES' >> Doxyfile
  fi

  set_doxy_key "WARNINGS" "YES" Doxyfile
  set_doxy_key "WARN_IF_DOC_ERROR" "YES" Doxyfile
  set_doxy_key "WARN_IF_INCOMPLETE_DOC" "YES" Doxyfile
  if [[ -z "$selected" ]]; then
    set_doxy_key "WARN_AS_ERROR" "FAIL_ON_WARNINGS" Doxyfile
  fi

  # Remove any existing TAGFILES lines before injecting ours
  inplace_sed '/^TAGFILES\b/d' Doxyfile

  for other in $doc_modules; do
    [[ "$other" == "$m" ]] && continue
    if [[ -f "$pages_rel/$other/$other.tag" ]]; then
      echo "TAGFILES += $pages_rel/$other/$other.tag=../$other" >> Doxyfile
    fi
  done

  doxygen Doxyfile

  popd >/dev/null

  cp -f "$css_src" "$pages_dir/$m/mydoxygen.css" 2>/dev/null || true
  [[ -f "$pages_dir/$m/index.html" ]] || die "HTML not produced for $m ($pages_dir/$m/index.html missing)"
  echo
done

# Build top-level index / landing page
if [[ -f "$script_dir/generate_html.py" ]]; then
  "$script_dir/generate_html.py"
elif [[ -f "./ci/generate_html.py" ]]; then
  ./ci/generate_html.py
fi

"$script_dir/check_doxygen_links.py" "$pages_dir"

echo "Done. Output in ./$pages_dir/"
