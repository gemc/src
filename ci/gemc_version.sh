#!/usr/bin/env bash
set -euo pipefail

# If HEAD is attached to a branch, this is a development checkout.
if git symbolic-ref -q HEAD >/dev/null; then
    echo dev
    exit 0
fi

# If HEAD is detached and exactly matches a tag, print the tag.
if tag=$(git describe --tags --exact-match HEAD 2>/dev/null); then
    echo "$tag"
    exit 0
fi

# Detached but not exactly a tag: still call it dev.
echo dev