# `meson/` — GEMC build dependency setup

This directory holds `meson.build`, included from the top-level build with `subdir('meson')`. It resolves every
external dependency GEMC links against and configures the bundled CMake subprojects (assimp, yaml-cpp). This
README documents how the three shared-with-CLAS12 dependencies — Qt6, CLHEP, and Geant4 — are loaded, and how
this repository acts as the **producer** of the Geant4 pkg-config files that downstream GEMC projects consume.

The companion consumer is `clas12-systems/meson/README.md`: read both together, because the Geant4 handling only
makes sense as a producer/consumer pair.


## Project defaults (upcoming in the next release)

The top-level `project()` sets the build defaults for both direct `meson setup` and CI. The former `core.ini`
native file is removed. The active settings are preserved: C++17, static GEMC libraries, static selection from
both-library targets, `prefer_static=true`, `b_lundef=false`, and `b_asneeded=false`. The existing `libdir=lib`
default is retained. The commented LTO, static PIC, and executable PIE settings are not added as defaults.

Builds that previously omitted the native file now use these settings too. `prefer_static=true` prefers archives
for dependencies without an explicit `static` choice; explicit shared requests such as Qt, Linux SQLite, and
zlib still take precedence. `b_lundef=false` permits unresolved plugin symbols at link time, and
`b_asneeded=false` retains shared dependencies even when the linker sees no direct symbol reference.

Override any default with a command-line `-D` option. CI still selects shared GEMC libraries for sanitizer and
profile builds. Existing build directories configured with `--native-file=core.ini` retain that path and need
a fresh setup after removal. Use a new build directory and repeat the desired prefix, build type, and feature
options, omitting `--native-file=core.ini`.


## Producer / consumer model

GEMC (`src`) is the root of the dependency graph. It is built and installed against a specific Geant4 / CLHEP,
and it records that choice in pkg-config files it installs into its own prefix:

- `src` **generates** `geant4.pc` and `geant4_core.pc` (Geant4 ships no pkg-config file of its own) and installs
  them into `<prefix>/lib/pkgconfig`.
- `src` also installs `gemc.pc` describing the GEMC libraries and their private static-link needs.
- Downstream projects (e.g. `clas12-systems`, which builds dlopen-able digitization plugins) do **not**
  re-derive Geant4 from `geant4-config`. They resolve `geant4_core` straight from the installed GEMC's
  `lib/pkgconfig`, so the plugins are always built against the exact Geant4 GEMC itself uses. See the consumer
  README for how that lookup finds the GEMC prefix from the `gemc` binary on `PATH`.

This is why the Geant4 code in the two repositories looks different: they play different roles. `src` is the
only place that runs `geant4-config` and builds the full `-lG4*` link line; `clas12-systems` only consumes the
resulting `.pc`.


## Qt6

Loaded with the `qt6` meson module. Qt is requested as **shared** (`static : false`) because supported package
managers ship Qt only as shared runtime libraries; asking for static archives just produces fallback warnings
and does not actually make Qt static. Includes are marked `include_type : 'system'` so Qt headers stay
warning-free.

- Required modules: `Core`, `Gui`, `Widgets`, `OpenGLWidgets` — the GUI cannot build without them.
- `Svg` is probed optionally. No GEMC source includes a QtSvg header; it is used only at run time to rasterize
  the toolbar button icons through the "SVG" image-format plugin. When missing, the GUI still builds and buttons
  fall back to empty icons — so it only warns.
- `Charts` is a real compile/link dependency (`gAnalysisView.cc` includes `<QtCharts/...>`). It is probed
  optionally: when present, `-DGEMC_HAS_QTCHARTS` is defined and the analysis view is built; when absent, GEMC
  still builds and the GUI analysis page shows a placeholder.

Qt6 exists only in `src`. `clas12-systems` has no GUI and therefore no Qt dependency at all.


## CLHEP

```meson
clhep_deps = dependency('clhep', version : '>=2.4.7.1', include_type : 'system')
```

Resolved through pkg-config (module name `clhep`), minimum version `2.4.7.1`, includes treated as system. It is
attached to essentially every GEMC module library and to the `gemc` binary.

CLHEP is loaded **shared** here — no `static : true`. This is deliberate and is the one intentional difference
from `clas12-systems`, which forces `static : true`:

- In `src`, GEMC and Geant4 must share a **single** copy of CLHEP's stateful globals — most importantly the
  `HepRandom` engine singleton that Geant4 uses for all random numbers. One shared `libCLHEP` guarantees
  exactly one RNG engine for the whole process.
- Forcing CLHEP static would be actively harmful in the shared / sanitizer / profile build (`use_sharedl`, see
  below): each of the ~30 GEMC module `.so`s would embed its own copy of CLHEP's globals, giving multiple
  diverging RNG states and triggering ASan/UBSan ODR-violation reports. That reintroduces exactly the
  duplicate-symbol class of problem `use_sharedl` exists to avoid.
- `clas12-systems` forces static for an unrelated, loader-specific reason: its plugins must carry no
  `@rpath/libCLHEP*` dependency or the `dlopen` fails. GEMC's executable has no such problem — it resolves
  `libCLHEP` at run time via Geant4's rpath.

Conclusion from the analysis: **do not make CLHEP static in `src`.** If a self-contained default binary were
ever an explicit goal, the only safe form would be conditional — `static : not use_sharedl` — keeping the
shared and sanitizer builds on shared CLHEP. The payoff is marginal, so the current shared setup stands.

`gemc.pc` still records `--libs --static clhep` under `libraries_private` regardless of this flag, so
downstream static-link information is present either way.


## Geant4

Geant4 ships no pkg-config file, so `src` builds its own and installs them for downstream use.

1. **Find `geant4-config`.** `$G4INSTALL/bin/geant4-config` is preferred when `$G4INSTALL` is set (the module
   layout), otherwise a `geant4-config` on `PATH`. Missing → hard error.
2. **Generate the `.pc` files.** `meson/g4_pkgconfig.py` writes `geant4.pc` and `geant4_core.pc` into the build
   tree (`<build>/pkgconfig`). They live in the build tree, not the install prefix, so a fresh `rm -rf build`
   regenerates them cleanly — important when switching Geant4 versions.
3. **Resolve them via pkg-config** with an augmented `PKG_CONFIG_PATH` (meson.build cannot mutate the
   `pkg_config_path` option or `dependency()`'s environment, so pkg-config is run directly and the result
   wrapped in `declare_dependency`). Both `.pc` are checked for `>=11.3.2` and for agreement with the active
   `geant4-config` version.
4. **Turn each `-lG4*` into a cached `cpp.find_library` object.** A shared cache makes the full set (`geant4`)
   and the core set (`geant4_core`) point at the same library objects. This lets meson deduplicate the Geant4
   archives across the whole link graph — raw `declare_dependency` link_args are never deduplicated, so with
   dozens of static modules each carrying the Geant4 link line, GNU ld would reprocess every large archive on
   each repeat and the link would crawl.
5. **Two dependency objects.** `geant4_dep` (full, `-isystem` includes) drives the GUI/vis targets;
   `geant4_core_dep` (core, plain `-I`) drives everything else. On Linux, `geant4_deps` also pulls in the
   X11/GLX stack for `G4OpenGL`.
6. **Install the `.pc` files** into `<prefix>/lib/pkgconfig` via an install script (they are generated
   build-tree files). This is the step that makes the producer/consumer model work: `clas12-systems` reads
   `geant4_core.pc` from here.


## `use_sharedl` — why there are two build shapes

```meson
use_sharedl = (sanitize != 'none') or (get_option('default_library') == 'shared')
```

- **Default build:** GEMC modules are static `.a` archives, linked once into the `gemc` executable. A static
  `link_with` library forwards its dependencies' link args, so plugins/examples receive only the
  compile/include part of `this_deps` to avoid duplicate `-l` flags.
- **`use_sharedl` build (sanitizer or `default_library=shared`, the profile/CI path):** each module is a
  self-contained shared library that does **not** forward those link args, so the full `this_deps` must be
  passed to plugins and examples instead.

Static linking of CLHEP and the bundled CMake subprojects interacts badly with the `use_sharedl` shape
(duplicate symbols / duplicated global state across shared objects), which is the core reason CLHEP is kept
shared — see the CLHEP section above.


## Other dependencies (brief)

- `opengl`, `sqlite3`, `expat` (Geant4's GIDI/XML parsing needs it under static Geant4), `zlib` (shared, to
  avoid linking the static system archive), `threads`.
- `ROOT` is optional: probed via `root-config`, wrapped in `declare_dependency`; disabled features degrade
  gracefully unless `-Droot=enabled` forces it.
- CMake subprojects `assimp` and `yaml-cpp` are configured through `cmake.subproject_options()`. yaml-cpp
  headers are part of GEMC's public API (`goption.h` includes `yaml-cpp/yaml.h`), so they are installed
  alongside the GEMC headers.
