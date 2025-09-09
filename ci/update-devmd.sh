#!/usr/bin/env bash
set -euo pipefail

echo "[devmd] start"

FILE="releases/dev.md"
START="<!-- AUTO-DEVMD:START -->"
END="<!-- AUTO-DEVMD:END -->"
mkdir -p "$(dirname "$FILE")"

# --- Branch detection (works in GH Actions and locally) ---
BRANCH="${GITHUB_REF_NAME:-$(git rev-parse --abbrev-ref HEAD || echo '')}"
if [ -z "$BRANCH" ] || [ "$BRANCH" = "HEAD" ]; then
  # Try to resolve origin/HEAD -> default branch
  DEFAULT_BRANCH="$(git symbolic-ref refs/remotes/origin/HEAD 2>/dev/null | sed 's#.*/##' || true)"
  BRANCH="${DEFAULT_BRANCH:-main}"
fi
echo "[devmd] branch = $BRANCH"

# --- Time window (optional) ---
SINCE="${DEVMD_SINCE:-}"   # e.g. 2024-01-01
UNTIL="${DEVMD_UNTIL:-}"   # e.g. 2025-09-01
FLAGS=""
[ -n "$SINCE" ] && FLAGS="$FLAGS --since=$SINCE"
[ -n "$UNTIL" ] && FLAGS="$FLAGS --until=$UNTIL"
echo "[devmd] since=${SINCE:-<none>} until=${UNTIL:-<now>}"

# --- Ensure file scaffold exists with both markers ---
if [ ! -f "$FILE" ]; then
  cat > "$FILE" <<'EOF'
# Dev Release Notes

<!-- AUTO-DEVMD:START -->
<!-- AUTO-DEVMD:END -->
EOF
  echo "[devmd] created $FILE scaffold"
fi

# --- Fetch history (no-op if local) ---
git fetch --all --tags --prune --quiet || true

# --- Build commit list ---
TMP_LOG="$(mktemp)"
# shellcheck disable=SC2086
if ! git log --date=short --no-show-signature \
     --pretty='- %ad **%h** — %s _(by %an)_' \
     $FLAGS \
     "$BRANCH" > "$TMP_LOG"; then
  echo "[devmd] WARN: git log failed; writing placeholder"
  echo "- (unable to read git log for $BRANCH)" > "$TMP_LOG"
fi

if ! [ -s "$TMP_LOG" ]; then
  echo "[devmd] no commits found; writing placeholder"
  echo "- (no commits in range)" > "$TMP_LOG"
fi

TMP_SECTION="$(mktemp)"
{
  printf "## Commits on %s" "$BRANCH"
  [ -n "$SINCE" ] && printf " since %s" "$SINCE"
  [ -n "$UNTIL" ] && printf " until %s" "$UNTIL"
  printf "\n\n"
  cat "$TMP_LOG"
  printf "\n"
} > "$TMP_SECTION"

# --- Replace the section between markers, preserving everything else ---
# We rebuild the file as: <before START> + START + <section> + END + <after END>
TMP_NEW="$(mktemp)"

# Extract "before" (everything up to START, excluding START)
BEFORE="$(sed -n "1,/${START}/p" "$FILE" | sed '$d' || true)"
# Extract "after" (everything from END to EOF, excluding END)
AFTER="$(sed -n "/${END}/,\$p" "$FILE" | sed '1d' || true)"

{
  [ -n "$BEFORE" ] && printf "%s\n" "$BEFORE"
  printf "%s\n" "$START"
  cat "$TMP_SECTION"
  printf "%s\n" "$END"
  [ -n "$AFTER" ] && printf "%s\n" "$AFTER"
} > "$TMP_NEW"

mv "$TMP_NEW" "$FILE"
chmod 0644 "$FILE"         # ensure normal read perms
echo "[devmd] wrote $(wc -l < "$FILE") lines to $FILE"
echo "[devmd] done"
