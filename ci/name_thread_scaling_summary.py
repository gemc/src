#!/usr/bin/env python3
"""Give a ThreadScale Markdown summary a machine-specific filename."""

from __future__ import annotations

import argparse
import json
import re
import unicodedata
from pathlib import Path
from typing import Any


def slug(value: object) -> str:
    """Return a lowercase filename component containing only ASCII letters, digits, and hyphens."""
    normalized = unicodedata.normalize("NFKD", str(value or "unknown"))
    ascii_value = normalized.encode("ascii", "ignore").decode("ascii").lower()
    return re.sub(r"[^a-z0-9]+", "-", ascii_value).strip("-") or "unknown"


def machine_identity(runner: dict[str, Any]) -> tuple[str, str, str, int]:
    """Extract CPU, OS, architecture, and physical-or-visible core count."""
    cpu = str(runner.get("cpu_model") or "unknown-cpu")
    platform = str(runner.get("platform") or runner.get("runner_os") or "unknown-os")
    release = str(runner.get("release") or "").strip()
    operating_system = f"{platform}-{release}" if release else platform
    architecture = str(runner.get("architecture") or runner.get("runner_arch") or "unknown-arch")
    cores = runner.get("physical_cores") or runner.get("visible_cpus") or runner.get("logical_cpus")
    if not isinstance(cores, int) or cores < 1:
        raise ValueError("ThreadScale runner metadata does not contain a positive core count")
    return cpu, operating_system, architecture, cores


def report_machine(report: dict[str, Any]) -> tuple[str, str, str, int]:
    """Require one machine identity across every benchmark in a report."""
    machines = {
        machine_identity(runner)
        for benchmark in report.get("benchmarks", [])
        for runner in benchmark.get("runners", [])
    }
    if not machines:
        raise ValueError("ThreadScale report does not contain runner metadata")
    if len(machines) != 1:
        raise ValueError(
            "ThreadScale report contains multiple machine configurations; create one summary per machine"
        )
    return machines.pop()


def machine_summary_name(report: dict[str, Any]) -> str:
    """Build summary_<cpu>_<os>_<arch>_<ncores>cores.md from report metadata."""
    cpu, operating_system, architecture, cores = report_machine(report)
    return (
        f"summary_{slug(cpu)}_{slug(operating_system)}_{slug(architecture)}_"
        f"{cores}cores.md"
    )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("report_directory", type=Path)
    args = parser.parse_args()

    report_directory = args.report_directory.resolve()
    report_file = report_directory / "scaling.json"
    source = report_directory / "summary.md"
    report = json.loads(report_file.read_text(encoding="utf-8"))
    destination = report_directory / machine_summary_name(report)
    if destination.exists():
        raise FileExistsError(f"Machine-specific summary already exists: {destination}")
    source.rename(destination)
    print(destination)


if __name__ == "__main__":
    main()
