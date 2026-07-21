#!/usr/bin/env python3

"""Reject duplicate Doxygen section anchors before generating module tag files."""

from __future__ import annotations

import argparse
from collections import defaultdict
from pathlib import Path
import re


ANCHOR_PATTERN = re.compile(
    r"(?:@|\\)(?:section|subsection|subsubsection|anchor)\s+([A-Za-z][A-Za-z0-9_]*)"
)
SOURCE_SUFFIXES = {".h", ".hh", ".hpp", ".hxx", ".c", ".cc", ".cpp", ".cxx", ".C", ".H"}


def collect_anchors(source_root: Path) -> dict[str, list[tuple[Path, int]]]:
    """Return Doxygen section anchors and the source locations defining them."""

    anchors: dict[str, list[tuple[Path, int]]] = defaultdict(list)
    source_files = (path for path in source_root.rglob("*") if path.suffix in SOURCE_SUFFIXES)
    for source_file in sorted(source_files):
        for line_number, line in enumerate(source_file.read_text(encoding="utf-8").splitlines(), start=1):
            match = ANCHOR_PATTERN.search(line)
            if match:
                anchors[match.group(1)].append((source_file, line_number))
    return anchors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source_root", nargs="?", default="gemc", type=Path)
    args = parser.parse_args()

    anchors = collect_anchors(args.source_root)
    duplicates = {anchor: locations for anchor, locations in anchors.items() if len(locations) > 1}
    if not duplicates:
        print(f"Doxygen anchor check passed: {len(anchors)} unique anchors")
        return 0

    print(f"Doxygen anchor check failed: {len(duplicates)} duplicate anchors")
    for anchor, locations in sorted(duplicates.items()):
        print(f"  {anchor}")
        for path, line_number in locations:
            print(f"    {path}:{line_number}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
