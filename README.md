<!-- TRANSLATIONS:START (managed by bin/translate) -->
**Languages:** **EN** · [IT](README.it.md) · [日本語](README.ja.md) · [FR](README.fr.md) · [中文](README.zh.md)
<!-- TRANSLATIONS:END -->

# GEMC

<table>
  <thead>
    <tr>
      <th width="33%">Quality</th>
      <th width="34%">Deploy</th>
      <th width="33%">Docs</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td align="center" valign="top">
        <a href="https://github.com/gemc/src/actions/workflows/test.yml">
          <img alt="Test" src="https://github.com/gemc/src/actions/workflows/test.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/sanitize.yml">
          <img alt="Sanitize" src="https://github.com/gemc/src/actions/workflows/sanitize.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/codeql.yml">
          <img alt="CodeQL" src="https://github.com/gemc/src/actions/workflows/codeql.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/thread_scaling.yml">
          <img alt="Thread Scaling"
               src="https://github.com/gemc/src/actions/workflows/thread_scaling.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/valgrind_profile.yml">
          <img alt="Valgrind Profile"
               src="https://github.com/gemc/src/actions/workflows/valgrind_profile.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/pygemc/actions/workflows/pygemc_tests.yml">
          <img alt="pygemc tests"
               src="https://github.com/gemc/pygemc/actions/workflows/pygemc_tests.yml/badge.svg">
        </a><br/>
        <a href="https://hub.docker.com/r/gemc/gemc">
          <img alt="Docker Pulls" src="https://img.shields.io/docker/pulls/gemc/gemc.svg">
        </a>
      </td>
      <td align="center" valign="top">
        <a href="https://github.com/gemc/src/actions/workflows/deploy.yml">
          <img alt="Deploy" src="https://github.com/gemc/src/actions/workflows/deploy.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/binary_tarballs.yml">
          <img alt="Binary Tarballs"
               src="https://github.com/gemc/src/actions/workflows/binary_tarballs.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/release_tarballs.yml">
          <img alt="Release Tarballs"
               src="https://github.com/gemc/src/actions/workflows/release_tarballs.yml/badge.svg?event=release">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/macos_tarball.yml">
          <img alt="macOS Tarball"
               src="https://github.com/gemc/src/actions/workflows/macos_tarball.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/dev_release.yml">
          <img alt="Nightly gemc Release"
               src="https://github.com/gemc/src/actions/workflows/dev_release.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/pygemc/actions/workflows/publish_pypi.yml">
          <img alt="Publish PyPI"
               src="https://github.com/gemc/pygemc/actions/workflows/publish_pypi.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/pygemc/actions/workflows/dev_release.yml">
          <img alt="Nightly pygemc Release"
               src="https://github.com/gemc/pygemc/actions/workflows/dev_release.yml/badge.svg">
        </a><br/>
        <a href="https://pypi.org/project/pygemc/">
          <img alt="pygemc PyPI" src="https://img.shields.io/pypi/v/pygemc.svg?cacheSeconds=300">
        </a>
      </td>
      <td align="center" valign="top">
        <a href="https://gemc.github.io/home/">
          <img alt="GEMC documentation" src="https://img.shields.io/badge/docs-gemc.github.io-blue.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/doxygen.yml">
          <img alt="Doxygen" src="https://github.com/gemc/src/actions/workflows/doxygen.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/home/actions/workflows/jekyll.yml">
          <img alt="Homepage" src="https://github.com/gemc/home/actions/workflows/jekyll.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/binder-tutorials/actions/workflows/publish-image.yml">
          <img alt="Publish Jupyter Image"
               src="https://github.com/gemc/binder-tutorials/actions/workflows/publish-image.yml/badge.svg">
        </a>
      </td>
    </tr>
  </tbody>
</table>

**GEMC** is a database-driven [Geant4](https://geant4.web.cern.ch) application for detector and
radiation-transport simulations. Its Python API lets you define complex detector setups without writing Geant4
code.

Detector descriptions stay separate from the executable: you create them in Python, store them in
ASCII or SQLite databases, and GEMC loads them at run time to execute the Geant4 simulation.

C++ plugins support custom detector response, magnetic fields, event generators, and output formats.

![GEMC architecture](https://gemc.github.io/home/assets/images/gemcArchitecture.svg)

<br/>

## Highlights

- Python-first detector definitions
- Geometry preview and VTK export with [`pyvista`](https://pyvista.org)
- ASCII, SQLite databases
- Geometry imports from GDML and CAD files
- Interactive Geant4/Qt visualization and off-screen image generation
- Built-in sensitive  types: `flux`, `gPhotonDetector`, `dosimeter`, `particle_counter`
- Simulation output in ASCII, CSV, JSON, and [ROOT](https://root.cern)
- Event generation from the GUI, command-line, YAML and LUND files
- Magnetic-field support, including multipoles field and ascii maps
- A Python `analyzer` module for reading and plotting simulation output (CSV, ROOT)
- Geometry variations and run-number-dependent configurations
- Meson-based C++ build with CI-tested Docker images for Linux `amd64` and `arm64`


<br/>

## Quickstart

The fastest way to try GEMC is through the examples:

- [Jupyter Quickstart](https://mybinder.org/v2/gh/gemc/binder-tutorials/main?urlpath=lab/tree/notebooks/basic/quickstart.ipynb)
- [Documentation and examples](https://gemc.github.io/home/)
- [Examples gallery](https://gemc.github.io/home/examples/)

A minimal local workflow that creates a `counter` detector and runs 1000 events throught it:

```shell
gemc-system-template -s counter
cd counter
./counter.py
gemc counter.yaml -n=1000
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
```

The template command creates a `counter` system,
the Python script builds the geometry and stores it in a SQLite file `gemc.db`,
`gemc` runs the simulation, and `gemc-analyzer` plots a variable from the CSV output.

<br/>

## Installation

For installation instructions see the [installation guide](https://gemc.github.io/home/installation/)

The Python environment uses Python 3.10-3.14 and includes `pymeshlab`. Upcoming in the next release:
the pygemc subproject selects a supported installed Python and manages both environments, including
dependency installation at `meson install` time.


<br/>

## pygemc

Detector systems are build and written to databases using [`pygemc`](https://pypi.org/project/pygemc/).
An example of definiting a sensitive flux box:

```python
from pygemc import GVolume, autogeometry

cfg = autogeometry("examples", "counter")

flux = GVolume("flux_box")
flux.description = "air flux box"
flux.make_box(40.0, 40.0, 2.0)
flux.set_position(0, 0, 100)
flux.material = "G4_AIR"
flux.color = "3399FF"
flux.digitization = "flux"
flux.set_identifier("box", 2)
flux.publish(cfg)
```

Geometry scripts can write SQLite or ASCII databases, display geometry with PyVista, or export VTK.js files:

```shell
./counter.py --factory sqlite
./counter.py -pv
./counter.py -pvvtk counter -pvz 0.25
```

<br/>

### PyVista Visualization

PyVista is part of the normal GEMC geometry workflow. It lets users inspect detector geometry before running
Geant4, export portable VTK.js scenes for documentation, and catch placement or rotation mistakes while editing
Python geometry scripts. Some examples:
<table>
  <tr>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/b2/b2.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/b2/gemc_view.png"
             alt="B2 PyVista" width="180"><br>
        Geant4 basic/b2
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/materials/material.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/materials/gemc_view.png"
             alt="Materials PyVista" width="180"><br>
        Materials
      </a>
    </td>
  </tr>
  <tr>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/scintillator_barrel/scintillator_barrel.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/scintillator_barrel/gemc_view.png"
             alt="Scintillator Barrel PyVista" width="180"><br>
        Scintillator Barrel
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/dc/dc.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/dc/gemc_view.png"
             alt="Materials PyVista" width="180"><br>
        CLAS12 DC
      </a>
    </td>
  </tr>
  <tr>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/boolean_solids/boolean_solids.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/boolean_solids/gemc_view.png"
             alt="Boolean Solids PyVista" width="180"><br>
        Boolean Solids
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/cad/cad.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/cad/gemc_view.png"
             alt="CAD Import PyVista" width="180"><br>
        CAD Import
      </a>
    </td>
  </tr>
</table>

Open the linked interactive PyVista scenes generated from the GEMC examples.


<br/>

## Running Simulations

GEMC is configured with YAML files and equivalent command-line options. A compact steering file can define the run
number, event count, generator, geometry systems, output streamers, and world volume:

```yaml
runno: 1
n: 100
nthreads: 1

gparticle:
  - name: proton
    p: 1500
    vz: -5.0

gsystem:
  - name: counter
    factory: sqlite

gstreamer:
  - filename: counter
    format: csv
  - filename: counter
    format: root

root: G4Box, 15*cm, 15*cm, 15*cm, G4_AIR
```

Run in batch mode:

```shell
gemc counter.yaml
```

Run with the Geant4 GUI:

```shell
gemc counter.yaml -gui
```

Execute a Geant4 macro text file (upcoming in the next release):

```shell
gemc counter.yaml -geant4_macro=run.mac
gemc counter.yaml -gui -geant4_macro=vis.mac
```

The equivalent YAML setting is `geant4_macro: run.mac`. GEMC executes the macro after initialization and
visualization setup, before entering the GUI or interactive terminal. Relative paths use the working directory;
Geant4 handles comments, aliases, and nested `/control/execute` commands. Missing files and failed commands
produce a nonzero exit status.

In batch mode, the macro controls event generation with commands such as `/run/beamOn 100`; GEMC does not
automatically run the `-n` events or the `-run_weights` allocation afterward. Detector conditions are prepared
using `-run`. A visualization-only macro generates no events unless it contains a beam-on command. Without
`geant4_macro`, the usual event processing is unchanged.

Generate an off-screen Geant4 image:

```shell
gemc counter.yaml -n=10 \
  -g4view="[{driver: TOOLSSG_OFFSCREEN, segsPerCircle: 200}]" \
  -g4camera="[{phi: -10*deg, theta: 250*deg}]" \
  -g4light="[{phi: 160*deg, theta: 120*deg}]"
```

<br/>

## Output and Analysis

The output layer is plugin-based. Built-in streamer formats include:

| Format  | Notes                                                                                                            |
|---------|------------------------------------------------------------------------------------------------------------------|
| `ascii` | Human-readable text output                                                                                       |
| `csv`   | Per-thread CSV tables for digitized hits, true information, generated particles, and tracked generated particles |
| `json`  | Structured event output                                                                                          |
| `root`  | ROOT TTrees, available when GEMC is built with ROOT                                                              |

Upcoming in the next release, track provenance can be enabled for event output:

```shell
gemc counter.yaml -save_original_track
gemc counter.yaml -save_all_ancestors
```

`-save_original_track` populates `otid` in each true-information hit. `-save_all_ancestors` also enables
original-track IDs and writes a deduplicated `ancestors` bank containing each hit-producing track and its
complete parent chain. CSV output uses a separate `_ancestors.csv` file; ASCII, JSON, and ROOT include the bank
in their event output.


`pygemc` provides some utitlities to analyze CSV or ROOT output. For example:

```shell
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
gemc-analyzer out.root E --kind root --detector flux --save energy.png
```


For example, after running the [`b2` example](https://gemc.github.io/home/examples/basic/b2/) with
`gemc b2.yaml -n=1000`, the analyzer produces:

<table>
  <tr>
    <td align="center">
      <img src="https://gemc.github.io/home/assets/images/examples/b2/analyzer_totEdep.png" width="100%"/>
      <br/>
      <sub><code>gemc-analyzer b2_t0_digitized.csv totEdep --kind csv</code></sub>
    </td>
    <td align="center">
      <img src="https://gemc.github.io/home/assets/images/examples/b2/analyzer_true_energy.png" width="100%"/>
      <br/>
      <sub><code>gemc-analyzer b2_t0_true_info.csv E --kind csv --data true_info</code></sub>
    </td>
  </tr>
</table>

<br/>

## Documentation

- [GEMC homepage](https://gemc.github.io/home/)
- [C++ logging](https://gemc.github.io/home/documentation/api/logging/) — numbered message templates,
  filtering, and error handling; template methods are upcoming in the next release
- [Thread scaling](https://gemc.github.io/home/documentation/support/thread_scaling) — multithread benchmarking
  and the scaling sweep
- [Profiling](https://gemc.github.io/home/documentation/support/valgrind_profile) — Valgrind callgrind metrics
  and how to read them
- [Python API repository](https://github.com/gemc/pygemc)
- [CLAS12 GEMC systems repository](https://github.com/gemc/clas12-systems)


<br/>

## Contributing

Contributions are welcome through normal pull requests. Before opening a pull request, run the relevant Meson tests and
keep changes focused. See:

- [`CONTRIBUTING.md`](CONTRIBUTING.md)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md)
- [`SECURITY.md`](SECURITY.md)
- [CI workflow guide](.github/workflows/README.md)

Upcoming in the next release: C++ logging methods accept complete messages with numbered parameters:

```cpp
log->info(0, "Using %1 threads out of %2 available cores.", useThreads, ncores);
```

Migrate concatenating calls in external plugins and rebuild against matching GEMC headers. The transitional
`*Template()` methods remain forwarding aliases. See the
[logging guide](https://gemc.github.io/home/documentation/api/logging/) for placeholder rules and handling
dynamic diagnostic text.

English logging catalogs are generated from the C++ templates in each owning repository. See
[message catalogs](translations/README.md) for extraction, validation, and installation commands. Logging
detects the language automatically; `-language=it` selects Italian and `-language=en` forces English.
Missing translations fall back to English. The shared Claude/Codex workflow generates project-language
catalogs through the same weekday cron job used for website and README translations.

Upcoming in the next release: GUI controls, tooltips, geometry descriptions, analyzer messages, and SVG button
captions use the same language selection. See [GUI translations](translations/gui/README.md). Qt Linguist tools
compile the GUI catalogs during the build; standard-dialog translations use Qt's installed language data.

<br/>

## Citation

If you use GEMC in scientific work, cite:

> M. Ungaro and the CLAS Collaboration, "The CLAS12 Geant4 simulation," *Nuclear Instruments and Methods
> in Physics Research Section A* **959**, 163422 (2020). https://doi.org/10.1016/j.nima.2020.163422

BibTeX and additional citation formats are in [`CITATION.md`](CITATION.md).

<br/>

## License

GEMC is licensed under the [Apache License, Version 2.0](LICENSE); see [`NOTICE`](NOTICE) for attribution and
third-party acknowledgments. The software also depends on separately licensed third-party components, including
Geant4, CLHEP, Qt, ROOT, SQLite, and Assimp.

<br/>

## Roadmap

See the [project roadmap](https://github.com/orgs/gemc/projects/1/views/4) for the current development plans.
