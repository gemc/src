"""Exercise macro startup through the GEMC executable, with all output in temporary directories."""

import os
from pathlib import Path
import subprocess
import sys
import tempfile


def main():
    executable = str(Path(sys.argv[1]).resolve())
    with tempfile.TemporaryDirectory(prefix="gemc-macro-") as temporary:
        workdir = Path(temporary)
        common = [
            executable,
            "-nthreads=1",
            "-n=7",
            "-run=12",
            "-gstreamer=[]",
            "-gparticle=[{name: geantino, p: 1*MeV}]",
            "-log_every=1",
            "-verbosity.gemc=2",
        ]

        def run(*arguments, success=True, events=0, contains=(), absent=()):
            result = subprocess.run(
                common + list(arguments),
                cwd=workdir,
                input="exit\n",
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=45,
                env={**os.environ, "G4RUN_MANAGER_TYPE": "Serial"},
            )
            output = result.stdout
            if result.returncode != (0 if success else 1):
                raise AssertionError(f"Unexpected exit status {result.returncode}:\n{output}")
            if output.count("Starting event n.") != events:
                raise AssertionError(f"Expected {events} events:\n{output}")
            macro_start = output.find("Executing Geant4 macro:")
            if macro_start >= 0:
                initialize = output.find("Executing UIManager command: /run/initialize")
                last_starter = output.rfind("Executing UIManager command:")
                if not 0 <= initialize <= last_starter < macro_start:
                    raise AssertionError(f"Macro must run after all starter commands:\n{output}")
            for text in contains:
                if text not in output:
                    raise AssertionError(f"Missing {text!r}:\n{output}")
            for text in absent:
                if text in output:
                    raise AssertionError(f"Unexpected {text!r}:\n{output}")

        (workdir / "child.mac").write_text(
            "# Native Geant4 aliases and nested macro execution\n"
            "/control/alias macro_marker NESTED_MACRO_OK\n"
            "/control/echo {macro_marker}\n"
        )
        macro = workdir / "run with spaces.mac"
        macro.write_text(
            "/control/execute child.mac\n"
            "/run/verbose 1\n"
            "/run/beamOn 2\n"
            "/control/echo MACRO_COMPLETE\n"
        )
        run(f"-geant4_macro={macro}", events=2,
            contains=("NESTED_MACRO_OK", "MACRO_COMPLETE", "Run 12 starts"))

        (workdir / "config.yaml").write_text("geant4_macro: run with spaces.mac\n")
        run("config.yaml", events=2, contains=("NESTED_MACRO_OK", "MACRO_COMPLETE"))

        (workdir / "setup.mac").write_text("/control/echo SETUP_ONLY\n")
        run("-geant4_macro=setup.mac", contains=("SETUP_ONLY",))
        run("-geant4_macro=setup.mac", "-i", contains=("SETUP_ONLY",))
        (workdir / "empty.mac").write_text("# An empty macro must not dispatch -n events.\n")
        run("-geant4_macro=empty.mac")

        run("-geant4_macro=missing.mac", success=False, contains=("Cannot read Geant4 macro",))
        run(f"-geant4_macro={workdir}", success=False, contains=("Cannot read Geant4 macro",))
        (workdir / "bad.mac").write_text("/gemc/nonexistent/command\n/control/echo UNREACHABLE\n")
        run("-geant4_macro=bad.mac", success=False,
            contains=("Geant4 macro failed",), absent=("UNREACHABLE",))
        (workdir / "nested_bad.mac").write_text("/control/execute bad.mac\n/control/echo UNREACHABLE\n")
        run("-geant4_macro=nested_bad.mac", success=False,
            contains=("Geant4 macro failed",), absent=("UNREACHABLE",))

        run("-n=3", events=3)
        print("Geant4 macro startup checks passed (10 cases).")


if __name__ == "__main__":
    main()
