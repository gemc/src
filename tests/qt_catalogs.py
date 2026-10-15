"""Offline format and extraction checks for Qt catalogs; lupdate is mocked for Python-only CI."""

from contextlib import redirect_stdout
import copy
import io
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest.mock import patch
import xml.etree.ElementTree as ET

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "ci"))
import qt_catalogs as catalogs


class QtCatalogTests(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        self.directory = self.root / "translations/gui"
        self.directory.mkdir(parents=True)
        (self.directory / "catalog.yaml").write_text("schema: 1\nlanguages: [it]\nsources: [gui]\n")
        (self.root / "gui").mkdir()
        (self.root / "gui/main.cc").write_text('tr("File %1");')
        self.key = ("Window", "File %1", "", False)

    def write(self, language, translations, keys=None):
        path = self.directory / (language + ".ts")
        path.write_text(catalogs.catalog_text(language, keys or translations, translations), encoding="utf-8")
        return path

    def test_round_trip_preserves_context_comment_unicode_and_exact_text(self):
        first = ("Window", " File %1\n", "disk", False)
        second = ("Other", " File %1\n", "remote", False)
        values = {first: (" ファイル %1\n",), second: (" Fichier %1\n",)}
        self.assertEqual(("it", values), catalogs.load(self.write("it", values)))

    def test_duplicate_context_source_comment_is_rejected(self):
        path = self.write("it", {self.key: ("File %1",)})
        tree = ET.parse(path)
        context = tree.getroot().find("context")
        context.append(copy.deepcopy(context.find("message")))
        tree.write(path, encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "duplicate Qt message"):
            catalogs.load(path)

    def test_placeholders_preserve_numbered_localized_and_plural_tokens(self):
        catalogs.validate_message("%L1 %2 %n %Ln", "%Ln %n %2 %L1 %2")
        for value in ("%1 %2 %n %Ln", "%L1 %n %Ln", "%L1 %2 %n %Ln %3"):
            with self.subTest(value=value), self.assertRaisesRegex(ValueError, "placeholders"):
                catalogs.validate_message("%L1 %2 %n %Ln", value)
        for invalid in ("%0", "%01", "%100"):
            with self.subTest(invalid=invalid), self.assertRaises(ValueError):
                catalogs.validate_message(invalid, invalid)

    def test_literal_percent_and_required_whitespace_are_preserved(self):
        source = " \t%1%%\nvalue\t "
        catalogs.validate_message(source, " \t%1%%\nvaleur\t ")
        for value in (source.strip(), source.replace("%%", "%"), source.replace("\n", " "),
                      source.replace("\t", " ")):
            with self.subTest(value=value), self.assertRaises(ValueError):
                catalogs.validate_message(source, value)

    def test_html_file_filters_and_keyboard_mnemonics_are_preserved(self):
        catalogs.validate_message("<b>&Run %1</b>", "<b>E&segui %1</b>")
        catalogs.validate_message("Logs (*.log);;All files (*)", "Log (*.log);;Tutti i file (*)")
        for source, value in [("<b>Run</b>", "<i>Esegui</i>"), ("&Run", "Esegui"),
                              ("Logs (*.log)", "Log (*.txt)"), ("A (*.log);;B (*)", "A (*.log);B (*)")]:
            with self.subTest(source=source), self.assertRaises(ValueError):
                catalogs.validate_message(source, value)

    def test_unfinished_entries_are_safe_fallback_and_completed_empty_entries_fail(self):
        path = self.write("it", {}, keys=[self.key])
        self.assertIsNone(catalogs.load(path)[1][self.key])
        tree = ET.parse(path)
        tree.getroot().find("context/message/translation").attrib.clear()
        tree.write(path, encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "empty completed"):
            catalogs.load(path)

    def test_plural_forms_follow_language(self):
        key = ("Window", "%n item(s)", "", True)
        for language, count in (("it", 2), ("fr", 2), ("ja", 1), ("zh", 1)):
            values = {key: ("%n item(s)",) * count}
            self.assertEqual(values, catalogs.load(self.write(language, values))[1])
        path = self.write("it", {key: ("%n item(s)", "%n item(s)")})
        tree = ET.parse(path)
        translation = tree.getroot().find("context/message/translation")
        translation.remove(translation.find("numerusform"))
        tree.write(path, encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "plural forms"):
            catalogs.load(path)

    def test_validation_requires_languages_metadata_and_current_context_keys(self):
        self.write("en", {self.key: ("File %1",)})
        with self.assertRaisesRegex(ValueError, "missing Qt catalog"):
            catalogs.validate(self.root)
        path = self.write("it", {self.key: ("File %1",)})
        path.write_text(path.read_text().replace('language="it"', 'language="fr"'))
        with redirect_stdout(io.StringIO()), self.assertRaisesRegex(ValueError, "normalized filename"):
            catalogs.validate(self.root)
        self.write("it", {("Other", "File %1", "", False): ("File %1",)})
        with redirect_stdout(io.StringIO()), self.assertRaisesRegex(ValueError, "unknown Qt"):
            catalogs.validate(self.root)

    def test_partial_validation_reports_fallback_without_modifying_files(self):
        self.write("en", {self.key: ("File %1",)})
        self.write("it", {}, keys=[self.key])
        before = {path: path.read_bytes() for path in self.directory.iterdir()}
        output = io.StringIO()
        with redirect_stdout(output):
            catalogs.validate(self.root)
        self.assertIn("Qt coverage it: 0/1", output.getvalue())
        self.assertEqual(before, {path: path.read_bytes() for path in self.directory.iterdir()})

    def test_extraction_excludes_examples_and_checks_semantics_without_writing(self):
        (self.root / "gui/examples").mkdir()
        (self.root / "gui/examples/example.cc").write_text('tr("Excluded");')
        (self.root / "gui/guiDoxy.h").write_text('tr("Excluded");')
        def lupdate(command, **kwargs):
            self.assertNotIn(str(self.root / "gui/examples/example.cc"), command)
            self.assertNotIn(str(self.root / "gui/guiDoxy.h"), command)
            output = Path(command[-1])
            output.write_text(catalogs.catalog_text("en", [self.key], {}))
            return types.SimpleNamespace(returncode=0, stdout="", stderr="")
        with patch.object(catalogs.subprocess, "run", side_effect=lupdate), redirect_stdout(io.StringIO()):
            catalogs.extract(self.root, "lupdate-mock")
            before = (self.directory / "en.ts").read_bytes()
            catalogs.extract(self.root, "lupdate-mock", check=True)
            self.assertEqual(before, (self.directory / "en.ts").read_bytes())
            self.write("en", {("Window", "Obsolete", "", False): ("Obsolete",)})
            with self.assertRaisesRegex(ValueError, "stale"):
                catalogs.extract(self.root, "lupdate-mock", check=True)

    def test_extraction_rejects_sources_outside_the_owning_repository(self):
        (self.directory / "catalog.yaml").write_text("schema: 1\nlanguages: [it]\nsources: [../external]\n")
        with self.assertRaises(ValueError):
            catalogs.extract(self.root, "unused")


if __name__ == "__main__":
    unittest.main()
