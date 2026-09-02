#!/usr/bin/env python3
"""Replace the generated ThreadScale section in the GEMC README."""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Optional


START_MARKER = "<!-- thread-scaling-results:start -->"
END_MARKER = "<!-- thread-scaling-results:end -->"


def demote_headings(markdown: str) -> str:
    lines = markdown.strip().splitlines()
    if lines and lines[0] == "# Thread Scaling Results":
        lines = lines[1:]
    return "\n".join(f"#{line}" if line.startswith("#") else line for line in lines).strip()


def update_readme(readme: str, summary: str, label: str, run_url: Optional[str] = None) -> str:
    if readme.count(START_MARKER) != 1 or readme.count(END_MARKER) != 1:
        raise ValueError("README must contain exactly one ThreadScale marker pair")
    start = readme.index(START_MARKER) + len(START_MARKER)
    end = readme.index(END_MARKER)
    if start >= end:
        raise ValueError("ThreadScale README markers are out of order")

    report = demote_headings(summary)
    source = f"[{label}]({run_url})" if run_url else label
    generated = f"\n\n_Latest {source}._\n\n{report}\n\n"
    return f"{readme[:start]}{generated}{readme[end:]}"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("readme", type=Path)
    parser.add_argument("summary", type=Path)
    parser.add_argument("--label", required=True)
    parser.add_argument("--run-url")
    args = parser.parse_args()

    updated = update_readme(
        args.readme.read_text(encoding="utf8"),
        args.summary.read_text(encoding="utf8"),
        args.label,
        args.run_url,
    )
    args.readme.write_text(updated, encoding="utf8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
