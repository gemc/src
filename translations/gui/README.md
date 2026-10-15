# Qt GUI translations

Upcoming in the next release: the GEMC GUI uses English, Italian, Japanese, French, or Chinese without a
language option. It shares the logging language selection: an explicit `-language=<code>` or YAML setting,
then `GEMC_LANGUAGE`, POSIX locale variables, and the native system language. `-language=en` forces English.
Unsupported languages and missing translations fall back to English. Restart GEMC to change the language.
Numeric formatting, scientific units, particle names, model identifiers, and Geant4 commands retain their
original meaning. Geant4's own guidance and output are supplied by Geant4.

## Authoring and generation

English C++ strings are the source of truth. Use `tr("Complete sentence with %1").arg(value)` for widget
text, and `QT_TRANSLATE_NOOP` with an explicit translation context for shared tables and SVG captions.
Keep command values in combo-box item data; never derive commands or application state from translated text.
Do not hand-edit the generated TS catalogs or translate numeric values and units.

From the owning GEMC source repository, with Qt 6 Linguist tools and PyYAML available:

```shell
build/subprojects/pygemc/python_env/bin/python ci/qt_catalogs.py extract
build/subprojects/pygemc/python_env/bin/python ci/qt_catalogs.py check
build/subprojects/pygemc/python_env/bin/python ci/qt_catalogs.py validate
```

`catalog.yaml` lists the owning GUI source directories. Extraction excludes build/subproject copies,
examples, and documentation snippets. Catalog keys include context, English text, disambiguation, and plural
status. The shared translation job handles plural forms and preserves Qt placeholders, rich text, keyboard
accelerators, file-filter patterns, and whitespace. Changing a context or sentence creates a new key.

## Shared translation workflow

The existing translation manifest enables `gemc/src-gui` with `layout: qt-catalog`. It uses the same languages,
glossary, agent translation and review, and weekday cron job as the websites, READMEs, and logging catalogs.
Run only the GUI entry with:

```shell
/opt/projects/cronjobs/update_translations.zsh gemc/src-gui
```

Each reviewed batch atomically saves TS output and progress. Per-file and per-message records store content
hashes and the full source git commit. Interrupted runs resume; obsolete keys are pruned. A state-file lock
serializes manual and scheduled runs so one process cannot overwrite another process's progress. Outputs
remain in the working tree for review and are not auto-committed.

## Build, installation, and fallback

Meson runs Qt `lrelease -nounfinished` to compile project-language TS files. QMs are embedded in the utilities
library and installed under `<prefix>/<datadir>/gemc/translations/gui`. Embedding preserves translations when
an executable is relocated. English source text supplies fallback without an English QM.

`gemc::makeQtApplication` installs translators before creating widgets. External `gui/<language>.qm` or
`<language>.qm` files in the normal catalog search paths, including `GEMC_TRANSLATION_PATH`, can override the
embedded catalogs. An exact regional catalog such as `it-it.qm` overrides the base language one message at a
time. Unfinished TS messages are excluded from QMs. Invalid QM files are ignored with a diagnostic.

Qt's installed `qtbase` catalogs translate standard dialogs when available. Linux build dependencies include
Qt Linguist tools; Qt translation data supplies standard-dialog translations. Homebrew Qt includes both.

## Validation

CI validates checked-in TS catalogs without calling agents or modifying files. The Qt-enabled Meson tests
also check extraction freshness and exercise actual widgets and dialogs in all project languages, automatic
selection, explicit English, unavailable and regional languages, invalid QMs, relocated resources, Unicode,
numbered counters, and command routing independent of labels. Layout checks cover run-button sizing; a full
visual review remains useful when new wording or widgets are added. Agent review checks linguistic accuracy.
