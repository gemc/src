"""Exercise real translated widgets, dialogs, command routing, counter updates, and relocated resources."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "ci"))
import qt_catalogs


def main():
    executable = Path(sys.argv[1]).resolve()
    source = Path(sys.argv[2]).resolve()
    lrelease = sys.argv[3]
    fields = {"run": ("GemcGUI", "Run"), "run_tooltip": ("GemcGUI", "Run events"),
              "counter_first": ("GemcGUI", "Event Number: %1"),
              "counter_final": ("GemcGUI", "Event Number: %1"),
              "dialog_title": ("GBoard", "Save Log File"),
              "dialog_filters": ("GBoard", "Log Files (*.log);;Text Files (*.txt);;All Files (*)")}
    with tempfile.TemporaryDirectory(prefix="gemc-gui-test-") as temporary:
        root = Path(temporary)
        def run(language, *options, binary=executable, locale="en_US.UTF-8", paths=""):
            environment = dict(os.environ)
            for name in ("GEMC_LANGUAGE", "GEMC_PLUGIN_PATH", "LC_MESSAGES"):
                environment.pop(name, None)
            environment.update({"QT_QPA_PLATFORM": "offscreen", "LC_ALL": locale,
                                "GEMC_TRANSLATION_PATH": paths})
            result = subprocess.run([str(binary), "-n=0", "-sql=missing.sqlite", "-language=" + language,
                                     "-gui_test_source=" + str(source), *options], cwd=root, env=environment,
                                    capture_output=True, text=True, timeout=30)
            if result.returncode:
                raise AssertionError(
                    f"GUI probe failed ({result.returncode}):\n{result.stdout}\n{result.stderr}")
            lines = [line for line in result.stdout.splitlines() if line.startswith("GUI_REPORT=")]
            if not lines:
                raise AssertionError(f"GUI report missing:\n{result.stdout}\n{result.stderr}")
            report = json.loads(lines[-1].removeprefix("GUI_REPORT="))
            assert report["physics_page"] and report["physics_button_fits"], report
            assert report["numeric_locale_unchanged"] and report["button_fits"] and report["caption_fits"]
            assert report["projection_command"] == "p 30", report
            assert report["culling_density"] == "0.001", report
            assert report["beam_command"] == "2", report
            assert report["dialog_found"], report
            return report

        def expected(report, language, regional_run=None):
            _, messages = qt_catalogs.load(source / "translations/gui" / (language + ".ts"))
            for field, (context, text) in fields.items():
                values = messages.get((context, text, "", False))
                value = values[0] if values else text
                if field == "run" and regional_run is not None:
                    value = regional_run
                if field.startswith("counter_"):
                    value = value.replace("%1", "3" if field == "counter_first" else "5")
                assert report[field] == value, (language, field, report[field], value)
            caption = messages.get(("GGuiIcons", "Generator", "", False))
            value = caption[0] if caption else "Generator"
            assert value in report["svg"] or value.replace("&", "&amp;") in report["svg"]

        for language in ("en", "it", "ja", "fr", "zh"):
            expected(run(language), language)
        expected(run("auto", locale="ja_JP.UTF-8"), "ja")
        expected(run("en", locale="it_IT.UTF-8"), "en")
        expected(run("zz"), "en")

        region = root / "regional"
        region.mkdir()
        key = ("GemcGUI", "Run", "", False)
        ts = region / "it-it.ts"
        ts.write_text(qt_catalogs.catalog_text("it-it", [key], {key: ("Regional run",)}))
        subprocess.run([lrelease, "-nounfinished", str(ts), "-qm", str(region / "it-it.qm")],
                       check=True, capture_output=True)
        expected(run("it_IT", paths=str(region)), "it", regional_run="Regional run")
        (region / "it-it.qm").write_bytes(b"invalid QM file")
        expected(run("it_IT", paths=str(region)), "it")

        relocated = root / "installed/bin"
        relocated.mkdir(parents=True)
        (relocated.parent / "lib").mkdir()
        copied = relocated / executable.name
        shutil.copy2(executable, copied)
        expected(run("fr", binary=copied), "fr")
    print("GUI languages, controls, dialogs, counter state, fallback, and relocated resources passed.")


if __name__ == "__main__":
    main()
