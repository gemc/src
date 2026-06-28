# GEMC

[![Test][test-badge]][test]
[![Sanitize][sanitize-badge]][sanitize]
[![CodeQL][codeql-badge]][codeql]
[![Deploy][deploy-badge]][deploy]
[![Binary Tarballs][binary-tarballs-badge]][binary-tarballs]
[![macOS Tarball][macos-tarball-badge]][macos-tarball]
[![Nightly Dev Release][nightly-badge]][nightly]
[![GEMC documentation][site-badge]][site]
[![pygemc PyPI][pypi-badge]][pypi]
[![Doxygen][docs-badge]][docs]
[![Docker Pulls][dockerhub-badge]][dockerhub]
[![Release Tarballs][release-tarballs-badge]][release-tarballs]
[![License][license-badge]][license]

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
- Geometry imports from databases, GDML, and CAD meshes
- Run-number and geometry variation support
- Built-in sensitive detector digitizations: `flux`, `gPhotonDetector`, `dosimeter`, `particle_counter`
- Dynamic C++ plugin infrastructure for custom digitization, fields, generators, and output streamers
- Event generation from command-line/YAML particle definitions and Lund files
- Output streamers for ASCII, CSV, JSON, JLAB SRO, and optional ROOT output
- Magnetic-field support, including multipoles field and ascii maps
- Interactive Geant4/Qt visualization and off-screen image generation
- PyVista geometry preview, VTK.js export
- Python analyzer for plotting CSV and ROOT output
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
- [Python API repository](https://github.com/gemc/pygemc)
- [CLAS12 GEMC systems repository](https://github.com/gemc/clas12-systems)


<br/>

## Contributing

Contributions are welcome through normal pull requests. Before opening a pull request, run the relevant Meson tests and
keep changes focused. See:

- [`CONTRIBUTING.md`](CONTRIBUTING.md)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md)
- [`SECURITY.md`](SECURITY.md)

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


[test]: https://github.com/gemc/src/actions/workflows/test.yml

[test-badge]: https://github.com/gemc/src/actions/workflows/test.yml/badge.svg

[deploy]: https://github.com/gemc/src/actions/workflows/deploy.yml

[deploy-badge]: https://github.com/gemc/src/actions/workflows/deploy.yml/badge.svg

[dockerhub]: https://hub.docker.com/r/gemc/gemc

[dockerhub-badge]: https://img.shields.io/docker/pulls/gemc/gemc.svg

[docs]: https://github.com/gemc/src/actions/workflows/doxygen.yml

[docs-badge]: https://github.com/gemc/src/actions/workflows/doxygen.yml/badge.svg

[sanitize]: https://github.com/gemc/src/actions/workflows/sanitize.yml

[sanitize-badge]: https://github.com/gemc/src/actions/workflows/sanitize.yml/badge.svg

[codeql]: https://github.com/gemc/src/actions/workflows/codeql.yml

[codeql-badge]: https://github.com/gemc/src/actions/workflows/codeql.yml/badge.svg

[nightly]: https://github.com/gemc/src/actions/workflows/dev_release.yml

[nightly-badge]: https://github.com/gemc/src/actions/workflows/dev_release.yml/badge.svg

[release-tarballs]: https://github.com/gemc/src/actions/workflows/release_tarballs.yml

[release-tarballs-badge]: https://github.com/gemc/src/actions/workflows/release_tarballs.yml/badge.svg

[binary-tarballs]: https://github.com/gemc/src/actions/workflows/binary_tarballs.yml

[binary-tarballs-badge]: https://github.com/gemc/src/actions/workflows/binary_tarballs.yml/badge.svg

[macos-tarball]: https://github.com/gemc/src/actions/workflows/macos_tarball.yml

[macos-tarball-badge]: https://github.com/gemc/src/actions/workflows/macos_tarball.yml/badge.svg

[site]: https://gemc.github.io/home/

[site-badge]: https://img.shields.io/badge/docs-gemc.github.io-blue.svg

[pypi]: https://pypi.org/project/pygemc/

[pypi-badge]: https://img.shields.io/pypi/v/pygemc.svg?cacheSeconds=300

[license]: LICENSE

[license-badge]: https://img.shields.io/badge/license-Apache--2.0-blue.svg
