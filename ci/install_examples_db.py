#!/usr/bin/env python3
"""Generate the installed examples SQLite database for the GUI setup tab."""

from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path


EXAMPLES = {
    "basic": {
        "simple_flux": {"runs": ["1"], "variations": ["default"]},
        "b1": {"runs": ["1"], "variations": ["default", "test"]},
        "b2": {"runs": ["1", "11"], "variations": ["default", "alt"]},
        "b3": {"runs": ["1", "11"], "variations": ["default", "alt"]},
    },
    "optical": {
        "cherenkov": {"runs": ["1"], "variations": ["default", "CO2", "C4F10"]},
    },
}


def run_geometry_script(python: Path, script: Path, db: Path, args: list[str], env: dict[str, str]) -> None:
    subprocess.run(
        [str(python), str(script), "-f", "sqlite", "-sql", str(db), *args],
        cwd=script.parent,
        env=env,
        check=True,
    )


def main() -> int:
    if os.environ.get("GEMC_SKIP_EXAMPLES_DB_INSTALL") == "1":
        print(" > Skipping examples database generation (GEMC_SKIP_EXAMPLES_DB_INSTALL=1)")
        return 0

    prefix = Path(os.environ["MESON_INSTALL_PREFIX"])
    examples_dir = prefix / "examples"
    db = examples_dir / "gemc.db"
    python = prefix / "python_env" / "bin" / "python3"

    if not python.exists():
        print(f" > Cannot generate examples database: {python} was not found", file=sys.stderr)
        return 1

    db.unlink(missing_ok=True)

    env = os.environ.copy()
    env["PYTHONDONTWRITEBYTECODE"] = "1"

    print(f" > Generating examples database at {db}")
    for branch, examples in EXAMPLES.items():
        for example, config in examples.items():
            script = examples_dir / branch / example / f"{example}.py"
            if not script.exists():
                print(f" > Missing example geometry script: {script}", file=sys.stderr)
                return 1

            for variation in config["variations"]:
                run_geometry_script(python, script, db, ["-v", variation], env)

            for run in config["runs"]:
                run_geometry_script(python, script, db, ["-r", run], env)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
