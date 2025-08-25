#!/usr/bin/env sh

# Discover dockerfiles: dockerfiles/Dockerfile-gemc-<tag>-<os>
# Outputs (for GitHub Actions):
#   matrix: {"include":[{"file":..., "tag":..., "os":..., "latest":true|false}, ...]}
#   image : ghcr.io/<owner>/gemc
# Works on macOS (BSD utils), Linux, and GHA.
# Example:
# GITHUB_REPOSITORY_OWNER="gemc" ./ci/discover-docker-matrix.sh

set -eu

DIR="${1:-dockerfiles}"

# Lowercase owner without bash-4 features
OWNER="${GITHUB_REPOSITORY_OWNER:-unknown}"
OWNER_LOWER=$(printf '%s' "$OWNER" | tr '[:upper:]' '[:lower:]')
IMAGE="ghcr.io/$OWNER_LOWER/gemc"

# Collect matches (handle spaces via NUL separators)
TMP_TSV="$(mktemp)"
FOUND=0
# macOS/BSD find supports -print0; if not, fall back without it
if find "$DIR" -type f -name 'Dockerfile-gemc-*' -print0 >/dev/null 2>&1; then
  find "$DIR" -type f -name 'Dockerfile-gemc-*' -print0 \
  | while IFS= read -r -d '' F; do
      FOUND=$((FOUND+1)) # this won't persist outside subshell; we’ll recount later
      BASE=$(basename "$F")                # Dockerfile-gemc-dev3-ubuntu24
      REST=${BASE#Dockerfile-}             # gemc-dev3-ubuntu24
      case "$REST" in
        gemc-*-*) : ;;
        *) continue ;;
      esac
      REST2=${REST#gemc-}                  # dev3-ubuntu24
      TAG=${REST2%%-*}                     # dev3
      OS=${REST2#*-}                       # ubuntu24 (may contain dashes)
      # trailing number as weight (dev3 -> 3; dev -> 0)
      DIGITS=$(printf '%s\n' "$TAG" | sed -n 's/.*\([0-9][0-9]*\)$/\1/p')
      [ -z "$DIGITS" ] && DIGITS=0
      printf '%s\t%s\t%s\t%s\n' "$F" "$TAG" "$OS" "$DIGITS" >> "$TMP_TSV"
    done
else
  # Fallback (no -print0)
  find "$DIR" -type f -name 'Dockerfile-gemc-*' \
  | while IFS= read -r F; do
      BASE=$(basename "$F")
      REST=${BASE#Dockerfile-}
      case "$REST" in
        gemc-*-*) : ;;
        *) continue ;;
      esac
      REST2=${REST#gemc-}
      TAG=${REST2%%-*}
      OS=${REST2#*-}
      DIGITS=$(printf '%s\n' "$TAG" | sed -n 's/.*\([0-9][0-9]*\)$/\1/p')
      [ -z "$DIGITS" ] && DIGITS=0
      printf '%s\t%s\t%s\t%s\n' "$F" "$TAG" "$OS" "$DIGITS" >> "$TMP_TSV"
    done
fi

# If nothing was written, bail out clearly
if [ ! -s "$TMP_TSV" ]; then
  echo "No matching dockerfiles found under '$DIR' (expected Dockerfile-gemc-<tag>-<os>)." >&2
  echo "Examples: Dockerfile-gemc-dev3-ubuntu24, Dockerfile-gemc-dev3-fedora40" >&2
  echo "[local] matrix={\"include\":[]}"
  echo "[local] image=$IMAGE"
  exit 1
fi

# Build JSON with BSD/POSIX awk (no non-portable features)
JSON=$(
  awk -F '\t' '
    BEGIN { n=0 }
    {
      file=$1; tag=$2; os=$3; w=$4+0
      n++; F[n]=file; T[n]=tag; O[n]=os; W[n]=w
      if (!(os in max) || w>max[os]) max[os]=w
    }
    END {
      printf "{\"include\":["
      for (i=1; i<=n; i++) {
        latest = (W[i] == max[ O[i] ]) ? "true" : "false"

        f=F[i]; gsub(/\\/,"\\\\",f); gsub(/\"/,"\\\"",f)
        t=T[i]; gsub(/\\/,"\\\\",t); gsub(/\"/,"\\\"",t)
        o=O[i]; gsub(/\\/,"\\\\",o); gsub(/\"/,"\\\"",o)

        printf "%s{\"file\":\"%s\",\"tag\":\"%s\",\"os\":\"%s\",\"latest\":%s}",
               (i>1?",":""), f, t, o, latest
      }
      printf "]}"
    }
  ' "$TMP_TSV"
)

COUNT=$(wc -l < "$TMP_TSV" | tr -d '[:space:]')
rm -f "$TMP_TSV"

# Emit for Actions if available; else print locally
if [ -n "${GITHUB_OUTPUT:-}" ]; then
  {
    echo "matrix<<EOF"
    echo "$JSON"
    echo "EOF"
    echo "image=$IMAGE"
  } >> "$GITHUB_OUTPUT"
else
  echo "[local] matrix=$JSON"
  echo "[local] image=$IMAGE"
fi

echo "Discovered $COUNT dockerfile(s). Image base: $IMAGE"
