#!/usr/bin/env python3
"""Exercise real Cherenkov geometry and persistent workers without changing source-tree databases."""

import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    executable, source, mode = sys.argv[1:]
    executable = str(Path(executable).resolve())
    example = Path(source) / "examples/optical/cherenkov"
    with tempfile.TemporaryDirectory(prefix="gemc-physics-") as temporary:
        subprocess.run([sys.executable, str(example / "cherenkov.py"), "-f", "ascii"],
                       cwd=temporary, check=True)
        command = [executable, str(example / "cherenkov.yaml"),
                   "-gsystem=[{name: cherenkov, factory: ascii, variation: mediumIndexRadiator}]",
                   "-physics_test_mode=" + mode, "-check_overlaps=0"]
        with (Path(temporary) / "run.log").open("w+") as log:
            environment = os.environ.copy()
            if mode == "gui":
                environment["QT_QPA_PLATFORM"] = "offscreen"
            result = subprocess.run(command, cwd=temporary, text=True, stdout=log,
                                    stderr=subprocess.STDOUT, env=environment, timeout=240)
            log.seek(0)
            output = log.read()
        if result.returncode or "PHYSICS_SWITCHING_PASSED" not in output or "ProcMan114" in output:
            print(output)
            raise SystemExit(result.returncode or 1)
        for line in output.splitlines():
            if line.startswith("PHYSICS_"):
                print(line)


if __name__ == "__main__":
    main()
