"""Regression checks for C++ message extraction, catalog serialization, and translation validation."""

from contextlib import redirect_stdout
import io
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "ci"))
import extract_messages as catalogs


class CatalogTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name).resolve()

    def source(self, text, name="source.cc"):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
        return path

    def extract(self, text, name="source.cc"):
        return catalogs.extract_file(self.source(text, name), self.root, catalogs.RECEIVERS)

    def catalog(self, messages, language="it", name="it.yaml"):
        path = self.root / name
        data = {"schema": 1, "language": language, "messages": messages}
        path.write_text(catalogs.yaml.safe_dump(data, allow_unicode=True), encoding="utf-8")
        return path

    def test_comments_strings_and_other_receivers_are_not_calls(self):
        messages = self.extract(r'''
            // log->info("Comment %2", value);
            /* log->warning("Another comment"); */
            auto text = R"cpp(log->error(1, "Inside a string"))cpp";
            database->error("Another API");
            log->info(0, "Real %1", read(1, 2));
        ''')
        self.assertEqual(messages, {"Real %1": ["source.cc:6"]})

    def test_adjacent_literals_raw_literals_and_cpp_escapes(self):
        messages = self.extract(r'''
            logger.info("File " /* comment */ "%1:\n\t\"quoted\"\\", filename);
            log->warning(R"x(Line one
Line two: %1)x", value);
            log->info("\xC3\xA9\u00e9\U0001f600\141\?");
        ''')
        self.assertEqual(set(messages), {'File %1:\n\t"quoted"\\', 'Line one\nLine two: %1', 'éé😀a?'})

    def test_all_categories_aliases_and_nested_arguments(self):
        messages = self.extract('''
            log->info(level, "Run %1", run);
            log->infoTemplate("Run %1", run);
            log->debug(NORMAL, "Debug %1", function({1, 2}, [a, b] { return a + b; }));
            log->warning("Warning %1", value);
            log->error(code, "Error %1", value);
            log->criticalTemplate("Critical %1", value);
        ''')
        self.assertEqual(set(messages), {"Run %1", "Debug %1", "Warning %1", "Error %1", "Critical %1"})
        self.assertEqual(messages["Run %1"], ["source.cc:2", "source.cc:3"])

    def test_logger_lifecycle_messages(self):
        messages = self.extract('''
            void debug(debug_type type, std::string_view message);
            debug(CONSTRUCTOR, "%1 logger", logger_name);
            debug(DESTRUCTOR, "%1 logger", logger_name);
        ''', "glogger.h")
        self.assertEqual(messages, {"%1 logger": ["glogger.h:3", "glogger.h:4"]})

    def test_banner_templates_and_shared_formatter_forwarding(self):
        messages = self.extract('''
            catalog->format(" %1  version: %2", executable, colored(version));
            catalog->format(" Called from: %1", colored(path));
        ''', "goptions.cc")
        self.assertEqual(set(messages), {" %1  version: %2", " Called from: %1"})
        self.assertEqual(self.extract('catalog->format(message, args);', "glogger.h"), {})
        with self.assertRaisesRegex(ValueError, "placeholders do not match"):
            self.extract('catalog->format(" Called from: %2", path);', "goptions.cc")

    def test_opaque_and_numeric_messages_are_not_translated(self):
        self.assertEqual(self.extract('''
            log->info("%1", diagnostic);
            log->debug(NORMAL, "%1%2: %3", function, name, value);
            log->info("%1%%", percent);
        '''), {})

    def test_invalid_templates_fail_with_source_location(self):
        calls = ['log->info("Invalid %0", v);', 'log->info("Invalid %01", v);',
                 'log->info("Invalid %100", v);', 'log->info("Missing %2", v);',
                 'log->info("Unused %1", v, w);', 'log->warning(dynamicText);']
        for call in calls:
            with self.subTest(call=call), self.assertRaisesRegex(ValueError, r"source\.cc:1:"):
                self.extract(call)

    def test_cpp_line_continuation(self):
        self.assertEqual(set(self.extract('log->info("Line\\\ncontinued %1", value);')), {"Linecontinued %1"})

    def test_yaml_round_trip_preserves_exact_runtime_keys(self):
        messages = {
            ' Leading spaces, "quotes", colon: hash# and newline\n %1\t': ["source.cc:1"],
            "Unicode é 😀 %1%%": ["source.cc:2"],
            "URL " + "https://example.test/" + "a" * 250 + " %1": ["source.cc:3"],
            " " * 250 + "word %1": ["source.cc:4"],
            "Backslashes " + "\\" * 150 + " %1": ["source.cc:5"],
        }
        content = catalogs.render_catalog(messages)
        self.assertTrue(all(len(line) <= 112 for line in content.splitlines()))
        parsed = catalogs.yaml.safe_load(content)
        self.assertEqual(parsed["messages"], {message: message for message in messages})
        self.assertEqual(content, catalogs.render_catalog(dict(reversed(list(messages.items())))))

    def test_translation_allows_reordering_repetition_and_partial_coverage(self):
        english = self.catalog({"First %1, second %2": "First %1, second %2", "Other": "Other"},
                               language="en", name="en.yaml")
        translated = self.catalog({"First %1, second %2": "%2 poi %1 e %2"})
        with redirect_stdout(io.StringIO()):
            self.assertEqual(catalogs.validate(translated, english), {"First %1, second %2": "%2 poi %1 e %2"})

    def test_translation_rejects_missing_added_invalid_or_escaped_parameters(self):
        for translation in ("File", "File %1 %2", "File %0", "File %01", "File %100", "File %%1", ""):
            with self.subTest(translation=translation), self.assertRaises(ValueError):
                catalogs.validate(self.catalog({"File %1": translation}))
        english = self.catalog({"Known": "Known"}, language="en", name="en.yaml")
        with redirect_stdout(io.StringIO()), self.assertRaisesRegex(ValueError, "unknown English template"):
            catalogs.validate(self.catalog({"Unknown": "Sconosciuto"}), english)
        with self.assertRaisesRegex(ValueError, "differs from its key"):
            catalogs.validate(self.catalog({"Known": "Changed"}, language="en"))

    def test_duplicate_yaml_keys_are_rejected(self):
        path = self.source('schema: 1\nlanguage: it\nmessages:\n  "File": "Uno"\n  "File": "Due"\n', "it.yaml")
        with self.assertRaisesRegex(ValueError, "duplicate YAML key"):
            catalogs.validate(path)

    def test_percent_escapes_and_whitespace_are_preserved(self):
        source = " \tFile %1: %2%%\n\t"
        valid = " \tFichier %1 : %2%%\n\t"
        with redirect_stdout(io.StringIO()):
            catalogs.validate(self.catalog({source: valid}))
        for translated in (valid.replace("%%", "%"), valid.lstrip(), valid.rstrip(),
                           valid.replace(" : ", "\n: "), valid.replace(" : ", "\t: ")):
            with self.subTest(translated=translated), self.assertRaises(ValueError):
                catalogs.validate(self.catalog({source: translated}))

    def test_boolean_schema_is_rejected(self):
        path = self.source('schema: true\nlanguage: it\nmessages: {}\n', "it.yaml")
        with self.assertRaisesRegex(ValueError, "schema 1"):
            catalogs.validate(path)

    def catalog_tree(self):
        self.source('log->info("First %1, second %2", one, two); log->info("Other");', "src/main.cc")
        self.source('schema: 1\nlanguages: [it]\nsources: [src]\n', "translations/catalog.yaml")
        with redirect_stdout(io.StringIO()):
            catalogs.extract(self.root, check=False)
        return self.catalog({"First %1, second %2": "%2 poi %1 — 日本語"}, name="translations/it.yaml")

    def test_tree_reports_partial_coverage_and_does_not_modify_files(self):
        self.catalog_tree()
        before = {path: path.read_bytes() for path in self.root.rglob("*") if path.is_file()}
        output = io.StringIO()
        with redirect_stdout(output):
            catalogs.validate_tree(self.root)
        self.assertIn("Coverage it: 1/2; 1 messages use English fallback.", output.getvalue())
        self.assertEqual(before, {path: path.read_bytes() for path in self.root.rglob("*") if path.is_file()})

    def test_tree_requires_declared_catalogs_and_matching_language_metadata(self):
        path = self.catalog_tree()
        path.unlink()
        with redirect_stdout(io.StringIO()), self.assertRaisesRegex(ValueError,
                                                                   "missing language catalogs: it"):
            catalogs.validate_tree(self.root)
        self.catalog({"Other": "Autre"}, language="fr", name="translations/it.yaml")
        with redirect_stdout(io.StringIO()), self.assertRaisesRegex(ValueError, "metadata must match"):
            catalogs.validate_tree(self.root)

    def test_tree_checks_additional_regional_catalogs_and_rejects_unknown_keys(self):
        self.catalog_tree()
        self.catalog({"Other": "Altro"}, language="it-it", name="translations/it-it.yaml")
        with redirect_stdout(io.StringIO()):
            catalogs.validate_tree(self.root)
        self.catalog({"Obsolete": "Altro"}, language="it-it", name="translations/it-it.yaml")
        with redirect_stdout(io.StringIO()), self.assertRaisesRegex(ValueError, "unknown English template"):
            catalogs.validate_tree(self.root)
        (self.root / "translations/it-it.yaml").unlink()
        self.catalog({}, language="it_IT", name="translations/it_IT.yaml")
        with redirect_stdout(io.StringIO()), self.assertRaisesRegex(ValueError, "normalized language code"):
            catalogs.validate_tree(self.root)

    def test_tree_rejects_stale_english_catalog(self):
        self.catalog_tree()
        self.source('log->info("Changed message");', "src/main.cc")
        with self.assertRaisesRegex(ValueError, "catalog is stale"):
            catalogs.validate_tree(self.root)

    def test_owning_source_tree_exclusions_and_stale_catalog(self):
        self.source('log->info("A real message");', "src/main.cc")
        for name in ("src/build/generated.cc", "src/subprojects/vendor.cc", "src/docsDoxy.h",
                     "src/examples/example.cc"):
            self.source('log->info("Excluded %0", value);', name)
        manifest = 'schema: 1\nsources: [src]\nexclude: ["src/*Doxy.h", "src/examples/*"]\n'
        self.source(manifest, "translations/catalog.yaml")
        with redirect_stdout(io.StringIO()):
            catalogs.extract(self.root, check=False)
            catalogs.extract(self.root, check=True)
        output = catalogs.load_yaml(self.root / "translations/en.yaml")
        self.assertEqual(output["messages"], {"A real message": "A real message"})
        self.source('log->info("A changed message");', "src/main.cc")
        with self.assertRaisesRegex(ValueError, "catalog is stale"):
            catalogs.extract(self.root, check=True)
        with self.assertRaises(ValueError):
            catalogs.repository_path(self.root, "../outside")
        with self.assertRaisesRegex(ValueError, "owning source tree"):
            catalogs.repository_path(self.root, "src/subprojects/vendor.cc")


if __name__ == "__main__":
    unittest.main()
