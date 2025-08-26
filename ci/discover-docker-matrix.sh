#!/usr/bin/env sh
# Discover dockerfiles: dockerfiles/Dockerfile-gemc-<tag>-<os>
# Emits for GitHub Actions:
#   - matrix: {"include":[{"file":..., "tag":..., "os":..., "latest":true|false}, ...]}
#   - image : ghcr.io/<owner>/gemc
# Works on macOS (BSD utils), Linux, and GHA.

set -eu

DIR="${1:-dockerfiles}"

# Determine owner (prefer GITHUB_REPOSITORY_OWNER; fallback to GITHUB_REPOSITORY)
OWNER="${GITHUB_REPOSITORY_OWNER:-}"
if [ -z "$OWNER" ] && [ -n "${GITHUB_REPOSITORY:-}" ]; then
  OWNER="${GITHUB_REPOSITORY%%/*}"
fi
[ -z "$OWNER" ] && OWNER="unknown"
OWNER_LOWER=$(printf '%s' "$OWNER" | tr '[:upper:]' '[:lower:]')
IMAGE="ghcr.io/$OWNER_LOWER/gemc"

TMP_TSV="$(mktemp)"

# Glob-based enumeration (portable)
set -- "$DIR"/Dockerfile-gemc-*
if [ "$1" = "$DIR/Dockerfile-gemc-*" ]; then
  echo "No matching dockerfiles found under '$DIR' (expected Dockerfile-gemc-<tag>-<os>)." >&2
  echo "[local] matrix={\"include\":[]}"
  echo "[local] image=$IMAGE"
  exit 1
fi

for F in "$@"; do
  [ -f "$F" ] || continue
  BASE=${F##*/}                 # Dockerfile-gemc-dev3-ubuntu24
  REST=${BASE#Dockerfile-}      # gemc-dev3-ubuntu24
  case "$REST" in
    gemc-*-*) ;;
    *) continue ;;
  esac
  REST2=${REST#gemc-}           # dev3-ubuntu24
  TAG=${REST2%%-*}              # dev3
  OS=${REST2#*-}                # ubuntu24 (may contain dashes)

  # trailing digits in TAG as weight (dev3 -> 3; dev -> 0)
  DIGITS=$(printf '%s\n' "$TAG" | sed -n 's/.*\([0-9][0-9]*\)$/\1/p')
  [ -z "$DIGITS" ] && DIGITS=0

  printf '%s\t%s\t%s\t%s\n' "$F" "$TAG" "$OS" "$DIGITS" >> "$TMP_TSV"
done

# If nothing was written, bail out clearly
if [ ! -s "$TMP_TSV" ]; then
  echo "No usable dockerfiles after parsing." >&2
  echo "[local] matrix={\"include\":[]}"
  echo "[local] image=$IMAGE"
  exit 1
fi

# Build JSON with POSIX/BSD awk (avoid /\"/ to silence BSD awk warning)
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

        f=F[i]; gsub(/\\/,"\\\\",f); gsub(/"/,"\\\"",f)
        t=T[i]; gsub(/\\/,"\\\\",t); gsub(/"/,"\\\"",t)
        o=O[i]; gsub(/\\/,"\\\\",o); gsub(/"/,"\\\"",o)

        printf "%s{\"file\":\"%s\",\"tag\":\"%s\",\"os\":\"%s\",\"latest\":%s}",
               (i>1?",":""), f, t, o, latest
      }
      printf "]}"
    }
  ' "$TMP_TSV"
)

COUNT=$(wc -l < "$TMP_TSV" | tr -d '[:space:]')

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
echo "Dockerfiles found:"
awk -F '\t' '{print " - " $1}' "$TMP_TSV"

rm -f "$TMP_TSV"
