#!/usr/bin/env python3
"""Summarize callgrind profiles into a per-category CEst table for the workflow summary.

Each callgrind file is read through ``callgrind_annotate --inclusive=yes`` so every function's
cost includes its callees, then the inclusive cost of a set of category entry points is reported.

Cost is reported as CEst (Cycle Estimation), the same derived event KCachegrind / qcachegrind show:

    CEst = Ir + 10 * (I1mr + D1mr + D1mw) + 100 * (ILmr + DLmr + DLmw)

so the callgrind run must have been taken with ``--cache-sim=yes``. If the cache events are missing
the script falls back to plain Ir (instruction reads).

The digitization, SRO worker, and field categories are DISCOVERED, not hard-coded: every
``<Class>::digitizeHit``, ``<Class>::stream_hit``, and ``GField_<Type>::GetFieldValue`` present in the
profile is reported as its own row. Plugins added elsewhere (for example in clas12-systems) therefore
appear automatically without editing this file.

Categories use inclusive cost and can overlap or nest (field evaluation is part of track swimming,
digitization is called from event processing), so the percentages are not meant to add up to 100%.
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path

# Fixed category entry points: (label, compiled regex matched against the demangled function name).
FIXED_CATEGORIES = [
    ("Track swimming (field propagation)", re.compile(r"\bG4PropagatorInField::ComputeStep\b")),
    ("Hit collection (sensitive detector)", re.compile(r"\bGSensitiveDetector::ProcessHits\b")),
    ("SRO event processing (worker)", re.compile(r"\bGEventAction::stream_event\b")),
    ("Output writing", re.compile(r"\bGStreamer::publishEventData\b")),
]

# Discovered category families: (family label, regex capturing the class before a known method).
DISCOVERED_FAMILIES = [
    ("Digitization", re.compile(r"\b([A-Za-z_]\w*)::digitizeHit\b")),
    ("SRO stream_hit", re.compile(r"\b([A-Za-z_]\w*)::stream_hit\b")),
    ("Field evaluation", re.compile(r"\b(GField_[A-Za-z0-9_]*)::GetFieldValue\b")),
]

CACHE_L1 = ("I1mr", "D1mr", "D1mw")
CACHE_LL = ("ILmr", "DLmr", "DLmw")


def cest(counts: dict[str, int]) -> int:
    """Cycle estimation from raw callgrind event counts (KCachegrind formula)."""
    l1 = sum(counts.get(event, 0) for event in CACHE_L1)
    ll = sum(counts.get(event, 0) for event in CACHE_LL)
    return counts.get("Ir", 0) + 10 * l1 + 100 * ll


def location_to_func(location: str) -> str:
    """Reduce a callgrind ``file:function [object]`` location to just the demangled function name."""
    location = re.sub(r"\s*\[[^\]]*\]\s*$", "", location).strip()
    # The location is 'file:function'. File paths carry no '::', while the function may, so the
    # separator is the first ':' that is not part of a '::' token.
    for index, char in enumerate(location):
        if char != ":":
            continue
        previous = location[index - 1] if index > 0 else ""
        following = location[index + 1] if index + 1 < len(location) else ""
        if previous != ":" and following != ":":
            return location[index + 1:]
    return location


def parse_annotate(text: str) -> tuple[dict[str, int], list[tuple[str, dict[str, int]]]]:
    """Parse ``callgrind_annotate`` output into (program totals, [(function, counts), ...])."""
    # callgrind_annotate may print a percentage after each count (e.g. "12,345 (6.7%)"); drop those
    # so the numeric columns tokenize cleanly.
    text = re.sub(r"\(\s*[\d.]+%\)", " ", text)

    events: list[str] | None = None
    for line in text.splitlines():
        if line.startswith("Events shown:"):
            events = line.split(":", 1)[1].split()
            break
    if not events:
        raise ValueError("no 'Events shown:' header in callgrind_annotate output")

    number = re.compile(r"^[\d,]+$|^\.$")
    total: dict[str, int] | None = None
    rows: list[tuple[str, dict[str, int]]] = []
    for line in text.splitlines():
        tokens = line.split()
        if len(tokens) <= len(events):
            continue
        count_tokens = tokens[: len(events)]
        if not all(number.match(token) for token in count_tokens):
            continue
        counts = {
            event: 0 if token == "." else int(token.replace(",", ""))
            for event, token in zip(events, count_tokens)
        }
        location = " ".join(tokens[len(events):])
        if location.endswith("PROGRAM TOTALS"):
            total = counts
            continue
        rows.append((location_to_func(location), counts))
    if total is None:
        raise ValueError("no 'PROGRAM TOTALS' row in callgrind_annotate output")
    return total, rows


def collect_rows(total_cest: int, rows: list[tuple[str, dict[str, int]]]) -> list[tuple[str, str, int]]:
    """Return [(label, entry symbol, CEst), ...] for every category found in the profile."""
    table: list[tuple[str, str, int]] = []

    for label, pattern in FIXED_CATEGORIES:
        matched = [(func, counts) for func, counts in rows if pattern.search(func)]
        value = sum(cest(counts) for _, counts in matched)
        symbol = pattern.pattern.replace(r"\b", "")
        table.append((label, f"`{symbol}`", value))

    for family, pattern in DISCOVERED_FAMILIES:
        discovered: dict[str, int] = {}
        method = pattern.pattern.split("::", 1)[1].replace(r"\b", "")
        for func, counts in rows:
            match = pattern.search(func)
            if match:
                discovered[match.group(1)] = discovered.get(match.group(1), 0) + cest(counts)
        for klass, value in sorted(discovered.items(), key=lambda item: item[1], reverse=True):
            table.append((f"{family}: {klass}", f"`{klass}::{method}`", value))

    table.sort(key=lambda item: item[2], reverse=True)
    return table


def render(title: str, total: dict[str, int], rows: list[tuple[str, dict[str, int]]]) -> str:
    total_cest = cest(total)
    lines = [f"### {title}", ""]
    lines.append(
        f"Program totals: **{total_cest / 1e6:,.1f} Mcycles** (CEst), "
        f"{total.get('Ir', 0) / 1e6:,.1f} Minstr (Ir). "
        "Categories use inclusive cost and may overlap, so they do not sum to 100%."
    )
    lines.append("")
    lines.append("| Category | Entry symbol(s) | CEst (Mcycles) | % of run |")
    lines.append("|----------|-----------------|---------------:|---------:|")
    for label, symbol, value in collect_rows(total_cest, rows):
        percent = 100.0 * value / total_cest if total_cest else 0.0
        lines.append(f"| {label} | {symbol} | {value / 1e6:,.1f} | {percent:.2f}% |")
    lines.append("")
    return "\n".join(lines)


def shorten(name: str, width: int = 90) -> str:
    """Escape table-breaking pipes and truncate a long demangled function name."""
    name = name.replace("|", "\\|")
    return name if len(name) <= width else name[: width - 1] + "…"


def top_routines(rows: list[tuple[str, dict[str, int]]], total_cest: int, count: int = 10) -> str:
    """Table of the hottest individual routines by self cost (time spent in the routine itself)."""
    ranked = sorted(
        ((func, cest(counts)) for func, counts in rows),
        key=lambda item: item[1],
        reverse=True,
    )
    lines = [f"### Top {count} routines by self time (CEst)", ""]
    lines.append("| # | Routine | CEst (Mcycles) | % of run |")
    lines.append("|---|---------|---------------:|---------:|")
    for rank, (func, value) in enumerate(ranked[:count], start=1):
        percent = 100.0 * value / total_cest if total_cest else 0.0
        lines.append(f"| {rank} | `{shorten(func)}` | {value / 1e6:,.1f} | {percent:.2f}% |")
    lines.append("")
    return "\n".join(lines)


def annotate(path: Path, inclusive: bool = True) -> str:
    """Run callgrind_annotate and return its stdout (raises CalledProcessError on tool failure).

    inclusive=True gives each function's cost including its callees (used for the category totals);
    inclusive=False gives self cost only (used to rank the hottest individual routines).
    """
    command = ["callgrind_annotate", "--threshold=100", str(path)]
    if inclusive:
        command.insert(1, "--inclusive=yes")
    return subprocess.run(
        command,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=True,
    ).stdout


def summarize(path: Path, title: str) -> str:
    incl_total, incl_rows = parse_annotate(annotate(path, inclusive=True))
    section = render(title, incl_total, incl_rows)
    try:
        self_total, self_rows = parse_annotate(annotate(path, inclusive=False))
        section += "\n" + top_routines(self_rows, cest(self_total))
    except (subprocess.CalledProcessError, ValueError) as error:
        section += f"\n_Top-routines table unavailable: {error}_\n"
    return section


def fallback(title: str, path: Path, detail: str) -> str:
    """A section that still names the problem, with collapsible diagnostics, when parsing fails."""
    return (
        f"### {title}\n\n_Profile summary unavailable for `{path.name}`._\n\n"
        f"<details><summary>diagnostics</summary>\n\n```\n{detail.strip()[:2000]}\n```\n\n</details>\n"
    )


def title_for(path: Path) -> str:
    name = path.name
    return name[len("callgrind.out."):] if name.startswith("callgrind.out.") else name


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("files", nargs="+", type=Path, help="callgrind output files")
    parser.add_argument("--title", help="section title (only when a single file is given)")
    args = parser.parse_args()

    sections = []
    for path in args.files:
        title = args.title if (args.title and len(args.files) == 1) else title_for(path)
        try:
            sections.append(summarize(path, title))
        except subprocess.CalledProcessError as error:
            detail = f"callgrind_annotate exited {error.returncode}:\n{(error.stderr or '').strip()}"
            print(f"Warning: {detail}", file=sys.stderr)
            sections.append(fallback(title, path, detail))
        except ValueError as error:
            # Re-run to capture the actual output so the format problem is visible in the summary.
            try:
                snippet = "\n".join(annotate(path).splitlines()[:30])
            except Exception as rerun_error:  # noqa: BLE001 - diagnostics only
                snippet = f"(could not re-run callgrind_annotate: {rerun_error})"
            detail = f"parse error: {error}\n\nFirst lines of callgrind_annotate output:\n{snippet}"
            print(f"Warning: could not parse {path}: {error}", file=sys.stderr)
            sections.append(fallback(title, path, detail))

    print("\n".join(sections))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
