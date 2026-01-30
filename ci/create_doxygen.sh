#!/usr/bin/env zsh
# Purpose:
#   - Generate an up-to-date base Doxyfile from `doxygen -g` (cached per doxygen version)
#   - Sanitize that base deterministically on every run (fixes template multi-line lists)
#   - Emit a tiny per-module Doxyfile that @INCLUDEs that sanitized base
#
# Usage:
#   ./ci/create_doxygen.sh <module_name>

set -euo pipefail

die() { print -u2 -- "ERROR: $*"; exit 2; }

(( $# == 1 )) || die "Usage: $0 <module_name>"
mod="$1"

script_dir="${0:A:h}"
cache_dir="$script_dir/doxygen/.cache"
mkdir -p "$cache_dir"

base_pure="$cache_dir/Doxyfile.pure"
base_file="$cache_dir/Doxyfile.base"
ver_file="$cache_dir/doxygen.version"

doxver="$(doxygen --version)"

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


# Replace a doxygen list key that may be written as a multi-line block:
#   KEY = *.a \
#         *.b \
# with a single-line canonical:
#   KEY = value
#
# We remove the KEY line and its following "indented *..." continuation lines,
# then append the canonical KEY line. Appending is semantically fine for doxygen.
replace_doxy_list_key_single_line() {
  local file="$1"
  local key="$2"
  local value="$3"

  # Remove existing KEY line + template continuation lines that follow it.
  awk -v key="^"key"[[:space:]]*=" '
    BEGIN{skip=0}
    $0 ~ key { skip=1; next }                   # drop the KEY = ... line
    skip && $0 ~ /^[[:space:]]+\*/ { next }     # drop indented "*.ext \" lines
    skip { skip=0 }                             # first non-continuation after block
    { print }
  ' "$file" > "$file.tmp" && mv "$file.tmp" "$file"

  # Defensive: remove any stray KEY lines that might still exist (shouldn't).
  if grep -qE "^${key}[[:space:]]*=" "$file"; then
    inplace_sed "/^${key}[[:space:]]*=/d" "$file"
  fi

  print -- "${key}            = ${value}" >> "$file"
}

sanitize_base() {
  local f="$1"

  # ---- normalize base settings (small/stable list) ----
  inplace_sed 's|^REPEAT_BRIEF[[:space:]]*=.*|REPEAT_BRIEF             = NO|g' "$f"
  inplace_sed 's|^OPTIMIZE_OUTPUT_FOR_C[[:space:]]*=.*|OPTIMIZE_OUTPUT_FOR_C     = YES|g' "$f"
  inplace_sed 's|^BUILTIN_STL_SUPPORT[[:space:]]*=.*|BUILTIN_STL_SUPPORT        = YES|g' "$f"
  inplace_sed 's|^EXTRACT_ALL[[:space:]]*=.*|EXTRACT_ALL              = YES|g' "$f"
  inplace_sed 's|^SOURCE_BROWSER[[:space:]]*=.*|SOURCE_BROWSER           = YES|g' "$f"
  inplace_sed 's|^RECURSIVE[[:space:]]*=.*|RECURSIVE                = YES|g' "$f"
  inplace_sed 's|^GENERATE_TREEVIEW[[:space:]]*=.*|GENERATE_TREEVIEW        = YES|g' "$f"
  inplace_sed 's|^FORMULA_FONTSIZE[[:space:]]*=.*|FORMULA_FONTSIZE         = 14|g' "$f"
  inplace_sed 's|^DOT_FONTNAME[[:space:]]*=.*|DOT_FONTNAME             = Avenir|g' "$f"
  inplace_sed 's|^DOT_FONTSIZE[[:space:]]*=.*|DOT_FONTSIZE             = 16|g' "$f"
  inplace_sed 's|^TEMPLATE_RELATIONS[[:space:]]*=.*|TEMPLATE_RELATIONS       = YES|g' "$f"
  inplace_sed 's|^GENERATE_LATEX[[:space:]]*=.*|GENERATE_LATEX           = NO|g' "$f"
  inplace_sed 's|^HAVE_DOT[[:space:]]*=.*|HAVE_DOT                = YES|g' "$f"
  inplace_sed 's|^UML_LOOK[[:space:]]*=.*|UML_LOOK                = YES|g' "$f"
  inplace_sed 's|^HIDE_FRIEND_COMPOUNDS[[:space:]]*=.*|HIDE_FRIEND_COMPOUNDS   = YES|g' "$f"
  inplace_sed 's|^QUIET[[:space:]]*=.*|QUIET                   = YES|g' "$f"
  inplace_sed 's|^WARNINGS[[:space:]]*=.*|WARNINGS                = YES|g' "$f"


  replace_doxy_list_key_single_line "$f" "FILE_PATTERNS" \
    "*.h *.hh *.hpp *.hxx *.c *.cc *.cpp *.cxx *.C *.H"

  replace_doxy_list_key_single_line "$f" "EXCLUDE_PATTERNS" \
    "*/.git/* */build/* */_build/* */cmake-build-*/* *.md *.markdown *.txt *.dox"

  # Force stylesheet setting (delete any existing, then append)
  inplace_sed '/^HTML_EXTRA_STYLESHEET[[:space:]]*=/d' "$f"
  print -- 'HTML_EXTRA_STYLESHEET    = mydoxygen.css' >> "$f"

  # Clear per-project knobs from the base (these will be set per-module)
  inplace_sed '/^PROJECT_NAME[[:space:]]*=/d' "$f"
  inplace_sed '/^INPUT[[:space:]]*=/d' "$f"
  inplace_sed '/^OUTPUT_DIRECTORY[[:space:]]*=/d' "$f"
  inplace_sed '/^HTML_OUTPUT[[:space:]]*=/d' "$f"
  inplace_sed '/^GENERATE_TAGFILE[[:space:]]*=/d' "$f"
  inplace_sed '/^TAGFILES[[:space:]]*=/d' "$f"
}

# ---- (re)generate base if missing or doxygen version changed ----
need_regen=0
if [[ ! -f "$base_pure" || ! -f "$base_file" || ! -f "$ver_file" ]]; then
  need_regen=1
elif [[ "$(cat "$ver_file")" != "$doxver" ]]; then
  need_regen=1
fi

if (( need_regen )); then
  print -- " [create_doxygen] Generating base for doxygen $doxver"
  doxygen -g "$base_pure" >/dev/null 2>&1
  cp -f "$base_pure" "$base_file"
  print -- "$doxver" > "$ver_file"
fi

# Always sanitize (fixes stale/broken cache contents too)
[[ -f "$base_file" ]] || die "Missing base file '$base_file'"
sanitize_base "$base_file"

# Emit tiny module Doxyfile that includes the generated base.
# Use absolute path so it works regardless of where the module lives.
base_abs="$base_file"

cat > Doxyfile <<EOF
@INCLUDE = $base_abs

PROJECT_NAME              = "$mod"
INPUT                     = .
OUTPUT_DIRECTORY          = ../pages
HTML_OUTPUT               = $mod
GENERATE_TAGFILE          = ../pages/$mod/$mod.tag
HTML_EXTRA_STYLESHEET     = mydoxygen.css

# TAGFILES is injected by ci/doxygen.sh in pass 2
EOF

# Optional: keep local copy for debugging/inspection
cp -f Doxyfile DoxyfilePure 2>/dev/null || true
