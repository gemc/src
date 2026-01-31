#!/usr/bin/env zsh
# File: ci/create_doxygen.sh
#
# Purpose:
#   - Generate a fresh base Doxyfile from `doxygen -g` on every run (NO .cache usage)
#   - Sanitize that base deterministically (fixes template multi-line lists)
#   - Emit a per-module Doxyfile that INLINES the sanitized base + module overrides
#
# Usage:
#   ./ci/create_doxygen.sh <module_name>

set -euo pipefail

die() { print -u2 -- "ERROR: $*"; exit 2; }

(( $# == 1 )) || die "Usage: $0 <module_name>"
mod="$1"

script_dir="${0:A:h}"

doxver="$(doxygen --version 2>/dev/null || true)"
[[ -n "$doxver" ]] || die "doxygen not found in PATH"

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
# Strategy:
#   - Remove KEY line and continuation lines that follow it
#   - Append canonical KEY line at end (Doxygen accepts last-one-wins)
replace_doxy_list_key_single_line() {
  local file="$1"
  local key="$2"
  local value="$3"

  # Remove existing KEY line + template continuation lines that follow it.
  # Continuations from `doxygen -g` are often indented and may start with "*.ext".
  awk -v key="^"key"[[:space:]]*=" '
    BEGIN{skip=0}
    $0 ~ key { skip=1; next }                       # drop the KEY = ... line
    skip && $0 ~ /^[[:space:]]+\*/ { next }         # drop indented "*..." lines
    skip && $0 ~ /^[[:space:]]+\*\.[^[:space:]]/ { next }  # (extra defensive)
    skip && $0 ~ /^[[:space:]]+[^#[:space:]].*\\$/ { next } # drop generic "\" continuation lines
    skip { skip=0 }                                 # first non-continuation after block
    { print }
  ' "$file" > "$file.tmp" && mv "$file.tmp" "$file"

  # Defensive: remove any stray KEY lines that might still exist.
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

  # Defensive: remove any stray lines that start with "*" (cause: broken multi-line list blocks)
  inplace_sed '/^[[:space:]]*\*/d' "$f"
}

# ---- Generate fresh base every run (no .cache usage) ----
tmp_pure="$(mktemp "${TMPDIR:-/tmp}/Doxyfile.pure.XXXXXX")"
tmp_base="$(mktemp "${TMPDIR:-/tmp}/Doxyfile.base.XXXXXX")"
cleanup() { rm -f -- "$tmp_pure" "$tmp_base"; }
trap cleanup EXIT

print -- " [create_doxygen] Generating base for doxygen $doxver"
doxygen -g "$tmp_pure" >/dev/null 2>&1
cp -f "$tmp_pure" "$tmp_base"

sanitize_base "$tmp_base"

# Emit a module Doxyfile by inlining the sanitized base, then overriding.
{
  cat "$tmp_base"
  cat <<EOF

# ---- module overrides ----
PROJECT_NAME              = "$mod"
INPUT                     = .
OUTPUT_DIRECTORY          = ../pages
HTML_OUTPUT               = $mod
GENERATE_TAGFILE          = ../pages/$mod/$mod.tag
HTML_EXTRA_STYLESHEET     = mydoxygen.css

# TAGFILES is injected by ci/doxygen.sh in pass 2
EOF
} > Doxyfile

# Optional: keep local copy for debugging/inspection
cp -f Doxyfile DoxyfilePure 2>/dev/null || true
