"""Check message formatting and compare canonical logging with its transitional aliases."""

import subprocess
import sys
import os


def run(executable, *arguments):
    return subprocess.run(
        [executable, *arguments], capture_output=True, text=True, timeout=30,
        env={**os.environ, "GEMC_LANGUAGE": "en"},
    )


def main():
    executable = sys.argv[1]
    result = run(executable)
    if result.returncode != 0:
        raise AssertionError(f"Template checks failed:\n{result.stdout}{result.stderr}")

    for mode in ("error", "invalid-level"):
        original = run(executable, mode)
        templated = run(executable, mode + "-template")
        if original.returncode == 0 or templated.returncode != original.returncode:
            raise AssertionError(f"{mode}: template changed the exit code: {templated}")
        if templated.stdout != original.stdout or templated.stderr != original.stderr:
            raise AssertionError(f"{mode}: template changed the output: {templated}")
        if mode == "error" and (
            templated.returncode != 42
            or "File missing%2.txt not found." not in templated.stderr
            or "Exit Code: 42" not in templated.stderr
        ):
            raise AssertionError(f"Fatal template lost the message or exit code: {templated}")

    print("Logger templates, filtering, and canonical/alias compatibility checks passed.")


if __name__ == "__main__":
    main()
