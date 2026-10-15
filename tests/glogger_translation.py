"""Exercise language precedence, catalog discovery, safe fallback, caching, and translated logger output."""

import os
from pathlib import Path
import shutil
import re
import subprocess
import sys
import tempfile

import yaml


MESSAGES = {
    "Using %1 threads.": "Uso di %1 thread.",
    "First %1, second %2.": "%2 poi %1 e ancora %2.",
    "File %1 was not found; using %2.": "Uso di %2: file %1 non trovato.",
    "Starting detector %1.": "Avvio del rivelatore %1.",
    "Loaded %1 entries from %2.": "Da %2 sono state caricate %1 voci.",
    "Verbose %1.": "Dettaglio %1.",
    "Cannot read file %1.": "Impossibile leggere il file %1.",
}


def write_catalog(directory, language="it", messages=None):
    directory.mkdir(parents=True, exist_ok=True)
    path = directory / (language + ".yaml")
    path.write_text(yaml.safe_dump({"schema": 1, "language": language,
                                   "messages": MESSAGES if messages is None else messages}, allow_unicode=True))
    return path


def main():
    executable = str(Path(sys.argv[1]).resolve())
    plugin = str(Path(sys.argv[2]).resolve())
    with tempfile.TemporaryDirectory() as temporary:
        root = Path(temporary)
        catalogs = root / "catalogs"
        write_catalog(catalogs)

        def run(*arguments, locale=None, translation_path=None, binary=executable):
            env = dict(os.environ)
            for name in ("GEMC_LANGUAGE", "GEMC_TRANSLATION_PATH", "GEMC_PLUGIN_PATH",
                         "LC_ALL", "LC_MESSAGES", "LANG"):
                env.pop(name, None)
            env.update({"GEMC_TRANSLATION_PATH": str(translation_path or catalogs), "LANG": "en_US.UTF-8"})
            env.update(locale or {})
            # A relocated test executable still needs build-tree libraries in shared/sanitized builds.
            for name in ("LD_LIBRARY_PATH", "DYLD_LIBRARY_PATH"):
                env[name] = str(Path(executable).parent.parent) + ":" + env.get(name, "")
            result = subprocess.run([binary, *arguments], cwd=root, env=env,
                                    text=True, capture_output=True, timeout=30)
            if result.returncode != (42 if "-test_mode=error" in arguments else 0):
                raise AssertionError(f"Unexpected exit code: {result}\n{result.stdout}\n{result.stderr}")
            return result

        def contains(result, *messages):
            for message in messages:
                assert message in result.stdout, f"Missing {message!r}:\n{result.stdout}\n{result.stderr}"

        translated = run("-language=it_IT.UTF-8", "-verbosity.translations=2", "-debug.translations=true")
        contains(translated, "LANGUAGE=it-it", "INFO L0: Uso di 4 thread.",
                 "8 poi literal %2 and %% e ancora 8.",
                 "Uso di fallback.txt: file missing%2.txt non trovato.", "Avvio del rivelatore dc.",
                 "DEBUG: Da dc%1.txt sono state caricate 10 voci.", "INFO L2: Dettaglio 9.",
                 "Untranslated 1.25.", "External diagnostic: %2 and %%")
        assert "INFO L0: Using 4 threads." not in translated.stdout
        contains(run("-language=it", "-test_mode=plugin", "-probe_plugin=" + plugin), "Uso di 6 thread.")
        write_catalog(catalogs, messages={**MESSAGES, " %1  version: %2": " VERSION TEST %2: %1"})
        banner = run("-v", "-language=it")
        plain = re.sub(r"\x1b\[[0-9;]*m", "", banner.stdout)
        assert re.search(r"^ VERSION TEST [^\n]+: test_glogger_translation$", plain, re.MULTILINE), plain
        quiet = run("-language=it")
        assert "Dettaglio 9." not in quiet.stdout and "DEBUG:" not in quiet.stdout
        fatal = run("-language=it", "-test_mode=error")
        assert "Impossibile leggere il file missing%2.txt." in fatal.stderr and "Exit Code: 42" in fatal.stderr

        # Explicit options override the environment; automatic mode observes POSIX locale precedence.
        contains(run("-language=en", locale={"GEMC_LANGUAGE": "it"}), "LANGUAGE=en", "Using 4 threads.")
        contains(run(locale={"GEMC_LANGUAGE": "it", "LC_ALL": "C"}), "LANGUAGE=it", "Uso di 4 thread.")
        contains(run(locale={"LC_ALL": "it_IT.UTF-8", "LC_MESSAGES": "en_US.UTF-8"}), "Uso di 4 thread.")
        contains(run(locale={"LC_MESSAGES": "it_IT.UTF-8"}), "Uso di 4 thread.")
        contains(run(locale={"LANG": "it_IT.UTF-8"}), "Uso di 4 thread.")
        contains(run(locale={"LC_ALL": "C.UTF-8", "LANG": "it_IT.UTF-8"}), "LANGUAGE=en", "Using 4 threads.")
        contains(run("-language=xx"), "LANGUAGE=xx", "Using 4 threads.")
        contains(run("-language=../../it"), "LANGUAGE=en", "Using 4 threads.")
        native = run(locale={"LANG": ""})
        assert re.search(r"LANGUAGE=[a-z0-9]+(?:-[a-z0-9]+)*\n", native.stdout)
        write_catalog(catalogs, "ja", {"Using %1 threads.": "%1 スレッドを使用します。"})
        contains(run(locale={"LANG": "ja_JP.UTF-8"}), "LANGUAGE=ja-jp", "4 スレッドを使用します。")

        # Regional entries win, while other messages come from the base-language catalog.
        regional = write_catalog(catalogs, "it-it", {"Using %1 threads.": "Regione: %1 thread."})
        contains(run("-language=it_IT"), "Regione: 4 thread.", "Avvio del rivelatore dc.")
        regional.unlink()
        contains(run("-language=it", "-test_mode=cache", "-rewrite=" + str(catalogs)), "Uso di 4 thread.")
        write_catalog(catalogs)

        # Invalid catalogs cannot replace valid English messages or alter fatal behavior.
        invalid = root / "invalid"
        for messages in ({"Using %1 threads.": "Missing parameter"}, {"Using %1 threads.": "Extra %1 %2"},
                         {"Using %1 threads.": "Escaped %%1"}, {"Using %1 threads.": "Invalid %0"},
                         {"Using %1 threads.": ""}):
            write_catalog(invalid, messages=messages)
            result = run("-language=it", translation_path=invalid)
            contains(result, "Using 4 threads.")
            assert "Ignoring invalid translation catalog" in result.stderr
        for text in ('schema: [broken',
                     'schema: 1\nlanguage: it\nmessages:\n  "Using %1 threads.": "A %1"\n'
                     '  "Using %1 threads.": "B %1"\n'):
            (invalid / "it.yaml").write_text(text)
            result = run("-language=it", "-test_mode=error", translation_path=invalid)
            assert "Cannot read file missing%2.txt." in result.stderr and "Exit Code: 42" in result.stderr

        # Discovery through existing plugin paths, then a relocatable installed bin/share layout.
        plugin_prefix = root / "plugins"
        plugin_lib = plugin_prefix / "lib"
        plugin_lib.mkdir(parents=True)
        write_catalog(plugin_prefix / "share/gemc/translations/clas12")
        absent = root / "absent"
        contains(run("-language=it", "-plugin_path=" + str(plugin_lib), translation_path=absent),
                 "Uso di 4 thread.")
        contains(run("-language=it", locale={"GEMC_PLUGIN_PATH": str(plugin_lib)}, translation_path=absent),
                 "Uso di 4 thread.")
        prefix = root / "installed"
        (prefix / "bin").mkdir(parents=True)
        installed_binary = str(prefix / "bin/test_glogger_translation")
        shutil.copy2(executable, installed_binary)
        write_catalog(prefix / "share/gemc/translations/gemc")
        contains(run("-language=it", translation_path=absent, binary=installed_binary), "Uso di 4 thread.")
        source = root / "source"
        build = source / "custom-build"
        (build / "bin").mkdir(parents=True)
        (build / "build.ninja").touch()
        write_catalog(source / "translations")
        build_binary = str(build / "bin/test_glogger_translation")
        shutil.copy2(executable, build_binary)
        contains(run("-language=it", translation_path=absent, binary=build_binary), "Uso di 4 thread.")
        print("Translated logging, locale selection, fallback, caching, and catalog discovery checks passed.")


if __name__ == "__main__":
    main()
