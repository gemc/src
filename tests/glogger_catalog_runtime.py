"""Smoke-test actual project catalogs with automatic locales, explicit overrides, and English fallback."""

import os
import re
from pathlib import Path
import subprocess
import sys
import tempfile

import yaml


def main():
    executable = str(Path(sys.argv[1]).resolve())
    directory = Path(sys.argv[2]).resolve()
    template = "Random engine >%1< not found. Exiting."
    value = "missing%2"
    manifest = yaml.safe_load((directory / "catalog.yaml").read_text(encoding="utf-8"))
    with tempfile.TemporaryDirectory(prefix="gemc-language-test-") as temporary:
        def run(language, *options):
            env = dict(os.environ)
            for name in ("GEMC_LANGUAGE", "GEMC_PLUGIN_PATH", "LC_MESSAGES"):
                env.pop(name, None)
            env.update({"GEMC_TRANSLATION_PATH": str(directory), "G4RUN_MANAGER_TYPE": "Serial",
                        "LC_ALL": language.replace("-", "_") + ".UTF-8", "LANG": "en_US.UTF-8"})
            result = subprocess.run([executable, "-nthreads=1", "-seed=12345", "-gstreamer=[]",
                                     "-randomEngine=" + value, *options], cwd=temporary, env=env,
                                    text=True, capture_output=True, timeout=30)
            if result.returncode != 2003 % 256:
                raise AssertionError(f"Unexpected exit: {result.returncode}\n{result.stdout}\n{result.stderr}")
            if "Ignoring invalid translation catalog" in result.stderr:
                raise AssertionError(result.stderr)
            requested = next((option.split("=", 1)[1] for option in reversed(options)
                              if option.startswith("-language=")), language)
            selected = requested if requested in manifest["languages"] else "en"
            check_banner(re.sub(r"\x1b\[[0-9;]*m", "", result.stdout), selected, include_plugin=False)
            return result.stderr

        def version(language, *options):
            env = dict(os.environ)
            env.update({"GEMC_LANGUAGE": "auto", "LC_ALL": language.replace("-", "_") + ".UTF-8",
                        "GEMC_TRANSLATION_PATH": str(directory), "GEMC_PLUGIN_PATH": "plugins%2"})
            result = subprocess.run([executable, *options], cwd=temporary, env=env,
                                    text=True, capture_output=True, timeout=30)
            assert result.returncode == 0, result.stderr
            return re.sub(r"\x1b\[[0-9;]*m", "", result.stdout)

        baseline = version("en", "-v")
        # Values are taken from the English banner; colors and labels may change, payloads must not.
        pairs = {}
        for label in ("Called from", "Install", "Released on", "Plugin path", "GEMC Reference",
                      "GEMC Homepage", "Author"):
            line = next(line for line in baseline.splitlines() if line.startswith(" " + label + ": "))
            pairs[" " + label + ": %1"] = (line.split(": ", 1)[1],)
        line = next(line for line in baseline.splitlines() if "  version: " in line)
        name, release = line.strip().split("  version: ", 1)
        pairs[" %1  version: %2"] = (name, release)

        def check_banner(output, language, include_plugin=True):
            messages = yaml.safe_load((directory / (language + ".yaml")).read_text())["messages"]
            for template, values in pairs.items():
                if template == " Plugin path: %1" and not include_plugin:
                    continue
                expected = messages.get(template, template)
                # A single regex pass avoids interpreting percent signs in parameter values.
                expected = re.sub(r"%([1-9][0-9]?)", lambda match: values[int(match[1]) - 1], expected)
                assert expected in output.splitlines(), (language, template, output)

        for language in manifest["languages"]:
            check_banner(version(language, "-v"), language)
            check_banner(version("en", "-v", "-language=" + language), language)
            check_banner(version("en", "-language=" + language, "--version"), language)
        check_banner(version("en", "-v", "-language=it_IT.UTF-8"), "it")
        assert version("it", "-v", "-language=en") == baseline
        assert version("it", "-v", "-language=zz") == baseline

        for language in manifest["languages"]:
            path = directory / (language + ".yaml")
            messages = yaml.safe_load(path.read_text(encoding="utf-8"))["messages"]
            expected = messages.get(template, template).replace("%1", value)
            assert expected in run(language), f"Automatic selection failed for {language}"
        english = template.replace("%1", value)
        assert english in run("it", "-language=en"), "Explicit English override failed"
        assert english in run("it", "-language=zz"), "Missing-language English fallback failed"
    print("Banner and logging language selection, parameters, overrides, and English fallback checks passed.")


if __name__ == "__main__":
    main()
