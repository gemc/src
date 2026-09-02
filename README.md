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
          <img alt="Nightly Dev Release"
               src="https://github.com/gemc/src/actions/workflows/dev_release.yml/badge.svg">
        </a><br/>
        <a href="https://pypi.org/project/pygemc/">
          <img alt="pygemc PyPI" src="https://img.shields.io/pypi/v/pygemc.svg?cacheSeconds=300">
        </a><br/>
        <a href="https://hub.docker.com/r/gemc/gemc">
          <img alt="Docker Pulls" src="https://img.shields.io/docker/pulls/gemc/gemc.svg">
        </a>
      </td>
      <td align="center" valign="top">
        <a href="https://gemc.github.io/home/">
          <img alt="GEMC documentation" src="https://img.shields.io/badge/docs-gemc.github.io-blue.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/doxygen.yml">
          <img alt="Doxygen" src="https://github.com/gemc/src/actions/workflows/doxygen.yml/badge.svg">
        </a>
      </td>
    </tr>
  </tbody>
</table>

GEMC, the GEant Monte-Carlo, is a database-driven [Geant4](https://geant4.web.cern.ch) application for detector and
radiation-transport
simulations. It keeps detector descriptions outside the compiled C++ application:

- users builds setups in Python
- GEMC loads those definitions at run time and executes the Geant4 simulation pipeline

The goal is to make Geant4-based simulation accessible to users who want to
prototype detector systems without writing a custom Geant4 application,
while still preserving C++ extension points for advanced detector response and output workflows.

![GEMC architecture](https://gemc.github.io/home/assets/images/gemcArchitecture.svg)

<br/>

## Highlights

- Python-first detector definitions through [`pygemc`](https://github.com/gemc/pygemc)
- Geometry and material storage in SQLite or ASCII databases
- Geometry imports from GDML, CAD meshes
- Run-number and geometry variations
- Built-in sensitive  types: `flux`, `gPhotonDetector`, `dosimeter`, `particle_counter`
- C++ plugin infrastructure for custom digitization, fields, generators, and output streamers
- Event generation from command-line/YAML particle definitions and LUND files
- Supported output formats: ASCII, CSV, JSON, ROOT
- Magnetic-field support, including multipoles field and ascii maps
- Interactive Geant4/Qt visualization and off-screen image generation
- PyVista geometry preview, VTK export
- Python analyzer CSV and ROOT formats
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

## Thread scaling

Upcoming in the next release, pull requests run a short thread-scaling sweep of representative basic and optical
examples. Weekly and versioned-release runs repeat full sweeps across hosted runners to report runtime, event
rate, speedup, and parallel efficiency. Each workflow artifact contains the complete CSV, JSON, and SVG reports
plus a plotted `summary_<cpu>_<os>_<arch>_<ncores>cores.md` report.

Developers can run the local multithreading and race-focused Meson suite. It detects the CPUs available to the
process, creates one sequential test for every thread count from one through that maximum, and can be repeated
to vary worker scheduling:

```shell
meson test -C build --suite threading --repeat 10 --print-errorlogs
```

To run the standard 20,000-event scintillator scaling sweep, make sure `gemc`, Git, Node.js 24 or newer, and
Python 3 are available, then run:

```shell
bin/scaling.sh
```

For example, select a 50,000-event workload, cap the sweep at 32 threads, and override ThreadScale's measurement
defaults with:

```shell
bin/scaling.sh --workload 50000 --max-threads 32 -- --runs 8 --warmup-runs 2 --summary-plots rate
```

The script clones the current ThreadScale development branch into a temporary directory and retains the full
report in `thread-scaling/`. Its Markdown report is named from the CPU model, OS and release, architecture, and
physical core count, for example `summary_amd-epyc-9354-32-core-processor_linux-5-14_x64_64cores.md`. The core
count falls back to CPUs visible to the benchmark when physical topology is unavailable. The script never reads
or modifies this README.

Set `THREADSCALE_REF` to test another branch or tag. Neither `thread-scaling/` nor `thread-scaling.parts/` may
already exist. The `{workload}` command placeholder ensures the simulated event count is the same value used to
calculate the reported rate. The powers-of-two sweep ends at the maximum number of CPUs visible to the process
unless `--max-threads` sets a cap. Options after `--` are passed to `test_scaling` after the defaults, so they
can override runs, warmups, duration, thread selection, strategy, replicas, and plot selection. Run
`bin/scaling.sh --help` for the wrapper options.

<br/>

## Documentation

- [GEMC homepage](https://gemc.github.io/home/)
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
