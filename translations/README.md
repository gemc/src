# Logging message catalogs

Upcoming in the next release: logging templates are extracted from the owning C++ repositories and resolved
through cached catalogs at runtime. Language selection is automatic, with English fallback for missing text.
The shared Claude/Codex workflow generates project-language catalogs through the existing weekday cron job.

Edit English templates in C++, then regenerate the catalog. Do not hand-edit `en.yaml` or its source-location
comments. `catalog.yaml` selects the source files; build directories and `subprojects/` copies are always
excluded. Documentation snippets, module examples, and tests are excluded by the GEMC manifest. The scanner
reads all conditional-compilation branches without invoking a C++ compiler.

From the GEMC source repository, use the build Python environment, which includes PyYAML:

```shell
build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py extract
build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py extract --check
build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py validate translations/en.yaml
build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py validate-tree
```

`extract` writes `translations/en.yaml`. `extract --check` fails if it needs updating and writes nothing;
the Meson `glogging_catalog_freshness` test runs this check. Extraction rejects nonliteral templates or
invalid/unused placeholders with a source location, and merges identical templates into a single entry.
Messages containing only placeholders, punctuation, or numbers are omitted: dynamic diagnostics and object
dumps remain parameter data. Templates containing literal words are included, even at debug verbosity.

The same tool generates a separate catalog for CLAS12 in its owning repository:

```shell
build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py extract --root ../clas12-systems
build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py extract --root ../clas12-systems --check
```

## Catalog format

Each language uses `translations/<language>.yaml`, with exact English templates as stable lookup keys:

```yaml
schema: 1
language: en
messages:
  # gemc/example.cc:10
  "Using %1 threads.": "Using %1 threads."
```

A translated catalog keeps the same keys and replaces only the values. Keys preserve original whitespace,
newlines, punctuation, and escapes; changing the English sentence creates a new key. Source locations are
comments for reviewers, separate from the message mapping. Long entries may use YAML's explicit `?` / `:`
mapping syntax and escaped line continuations to preserve the text while wrapping at column 112.

The source manifest uses schema 1 with a `sources` list and optional `exclude` patterns, relative to the
repository root. Exclusion patterns use Python `fnmatch` semantics, where `*` also matches directory
separators. The tool recognizes the `log`, `logger`, `glog`, `glogger`, and `caller_log` receiver names plus
the logger's own constructor/destructor calls. A manifest can set `receivers` to include additional logger
names. Canonical methods and the transitional `*Template` aliases use the same extraction rules.

Templates must use narrow string literals, optionally adjacent or raw. Parentheses, brackets, and braces in
parameters are balanced. Wrap expressions containing template-argument commas in parentheses so the scanner
can distinguish them from logging argument separators; this tool is not a complete C++ parser.

## Validating translations

Before accepting an agent-generated catalog, validate it against the current English catalog:

```shell
build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py validate \
    translations/it.yaml --source translations/en.yaml
```

Validation rejects duplicate YAML keys, unknown English keys, empty translations, malformed placeholders,
and missing or additional parameter positions. Translations may reorder or repeat parameters. `%%` remains
a literal percent escape, so `%%1` is not a reference to parameter 1. Partial catalogs use English fallback
at runtime. English catalog values must match their keys exactly.

## Runtime language selection

Users do not need a language option. The default `language: auto` selects the first applicable setting:

1. An explicit `-language=<code>` or YAML `language: <code>` setting.
2. `GEMC_LANGUAGE`, unless it is empty or `auto`.
3. The first nonempty POSIX locale variable: `LC_ALL`, `LC_MESSAGES`, then `LANG`.
4. The macOS preferred language or Windows user locale when those variables are unset; otherwise English.

For example, `it_IT.UTF-8` becomes `it-it`. A regional catalog (`it-it.yaml`) is searched before the base
catalog (`it.yaml`), allowing partial regional overrides. `C`, `C.UTF-8`, and `POSIX` select English.
An unsupported language or missing message uses the original English template. `-language=en` forces
English; `GLogger::get_language()` reports the normalized requested tag, even when individual messages
fall back. No process locale is changed, so numeric values and scientific data retain their formatting.

The startup banner uses the same catalogs for version, calling/installation paths, release date, reference,
homepage, author, and optional plugin-path labels. Values remain parameter data, including ANSI colors and
literal percent signs. `gemc -v -language=it` honors the language override regardless of argument order.
Version remains an early action and uses command-line/environment settings without parsing YAML files.


## Catalog installation and discovery

Meson installs language catalogs under `<prefix>/<datadir>/gemc/translations/gemc` for the framework and
`<prefix>/<datadir>/gemc/translations/clas12` for the plugins. The default `<datadir>` is `share`. Manifests
and the catalog README are excluded. Adding a language file does not require recompiling C++.

`GEMC_TRANSLATION_PATH` optionally supplies colon-separated directories before the installed catalog
directory. Each directory may hold language YAML files directly or one level below it in named domains.
Existing `-plugin_path` and `GEMC_PLUGIN_PATH` directories also expose catalogs in their neighboring
installation prefixes. Recognized build trees use the owning source repository's `translations` directory.
This works without selecting a catalog path for a normal installation.

For a given requested language, exact regional entries precede base-language entries. Within each variant,
search paths follow the order above, with direct files before alphabetically ordered domain directories;
the first matching message wins. Language and relative paths are resolved when a configuration is first
loaded. Matching loggers share an immutable cache, and message lookup does not perform file I/O. Restart
the process to pick up catalog changes or a different working directory for the same relative paths.

An invalid YAML file, duplicate key, empty value, or changed placeholder set causes the entire file to be
ignored with one diagnostic per cached configuration. Other catalogs and English fallback remain available,
including for fatal messages. Filtering, message headers, counters, colors, and exit codes are preserved.
Plugins and standalone consumers must rebuild against matching GEMC headers and the new `glogging` library.

## CI validation

Upcoming in the next release: `validate-tree` checks that English is current and validates every language
YAML file in the owning `translations` directory. The manifest's `languages` list declares required catalog
files; additional regional catalogs are checked too. Language metadata must match the normalized filename.
Duplicate or unknown keys, empty values, altered parameter positions, literal percent escapes, surrounding
whitespace, line breaks, and tabs fail validation. Missing messages are reported as coverage and keep English
fallback; they do not fail CI while the scheduled translation job catches up.

The Test workflow runs these checks in a separate Python job before the build matrix. It needs PyYAML and
does not call agents or change catalogs. GEMC also registers `glogging_project_catalogs` and
`glogging_project_languages` in Meson; the latter runs GEMC with each project language, an explicit English
override, and an unavailable language. Existing logger tests cover regional fallback and plugin discovery.
CLAS12's Python CI job uses the validator from the same GEMC `main` source used by its build dependency.
Language accuracy remains the responsibility of the translation/review agents and human review.

## Shared agent workflow

The existing `~/myenv/bin/translate/translate.py` tool and weekday cron job also process runtime catalogs.
`/opt/projects/cronjobs/to_translate.yaml` enables `gemc/src-messages` and `gemc/clas12-messages` with
`layout: catalog`, `file: translations/en.yaml`, and the owning extractor path. Languages and technical
terminology come from the same `languages.yaml` and `glossary.yml` used for the websites and READMEs.

The catalog handler refreshes English templates, sends batches to Claude, and requests Codex review.
Local checks reject changed placeholders, literal percent escapes, surrounding whitespace, line breaks,
and tabs. It atomically saves validated catalog output and state after every batch. Only changed/new
messages need translation; removed English keys are pruned. Source-location comments do not change the
semantic source hash. Per-file and per-message state record the full git commit alongside content hashes,
so uncommitted English changes are also identified by their content.

Run just the catalogs with the existing runner:

```shell
/opt/projects/cronjobs/update_translations.zsh gemc/src-messages gemc/clas12-messages
```

Partial outputs retain English fallback. Failed batches resume on the next run, and failures in another
repository do not prevent catalog entries from being attempted. `--force` refreshes every message;
`--no-validate` explicitly skips Codex review while preserving the mandatory local checks. The extraction
tool itself performs no agent calls. Translations land in the working trees and are not auto-committed.

## Qt GUI catalogs

Upcoming in the next release: GUI text uses context-aware TS catalogs in `gui/`, compiled into QM resources
with Qt Linguist tools. GUI and logging share automatic language detection and explicit overrides. See the
[GUI translation guide](gui/README.md) for authoring, the shared cron workflow, installation, and tests.
