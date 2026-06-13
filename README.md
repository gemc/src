# GEMC

[![Test][test-badge]][test]
[![Deploy][deploy-badge]][deploy]
[![Sanitize][sanitize-badge]][sanitize]
[![CodeQL][codeql-badge]][codeql]
[![Doxygen][docs-badge]][docs]
[![Nightly Dev Release][nightly-badge]][nightly]
[![Homepage][site-badge]][site]
[![pygemc PyPI][pypi-badge]][pypi]
[![License][license-badge]][license]

GEMC, the GEant Monte-Carlo, is a database-driven [Geant4](https://geant4.web.cern.ch) application for detector and radiation-transport simulations. It keeps detector descriptions outside the compiled C++ application: users define geometry, materials, optics, mirrors, sensitive detectors, and run configuration in Python or data files, then GEMC loads those definitions at run time and executes the full Geant4 simulation pipeline.

The project goal is to make Geant4-based simulation accessible to users who want to prototype detector systems without writing a custom Geant4 application, while still preserving C++ extension points for advanced detector response and output workflows.

![GEMC architecture](https://gemc.github.io/home/assets/images/gemcArchitecture.svg)

## Highlights

- Python-first detector definition through [`pygemc`](https://github.com/gemc/pygemc)
- Geometry and material storage in SQLite or GEMC ASCII databases
- Geometry imports from native GEMC databases, GDML, and CAD meshes
- Run-number and variation support for reusable detector configurations
- Built-in sensitive detector digitizations: `flux`, `gPhotonDetector`, `dosimeter`, and `particle_counter`
- Dynamic C++ plugin infrastructure for custom digitization, fields, particle readers, and output streamers
- Event generation from command-line/YAML particle definitions and Lund files
- Output streamers for ASCII, CSV, JSON, JLAB SRO, and optional ROOT output
- Magnetic-field support, including a built-in multipole field plugin
- Interactive Geant4/Qt visualization and off-screen image generation
- PyVista geometry preview, interactive Qt display, and VTK.js export from Python geometry scripts
- Python analyzer for plotting GEMC CSV and ROOT output
- Meson-based C++ build with CI-tested Docker images for Linux `amd64` and `arm64`


## Quickstart

The fastest way to try GEMC is through the hosted examples:

- [Documentation and examples](https://gemc.github.io/home/)
- [Installation guide](https://gemc.github.io/home/installation/)
- [Quickstart tutorial](https://gemc.github.io/home/documentation/quickstart/)
- [Examples gallery](https://gemc.github.io/home/examples/)

A minimal local workflow looks like this after installation:

```shell
gemc-system-template -s counter
cd counter
./counter.py
gemc counter.yaml
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
```

The template command creates a small detector system, the Python script writes `gemc.db`, `gemc` runs the simulation, and `gemc-analyzer` plots a variable from the CSV output.

## Installation

Choose the installation path based on what you need:

- Use the [PyPI `pygemc` package](https://pypi.org/project/pygemc/) for Python geometry building, PyVista previews, and output analysis without the compiled `gemc` simulator.
- Build GEMC from source for the full Geant4 simulation executable and the bundled `pygemc` Python environment.
- Use Docker when you want a ready-to-run development environment without managing local Geant4 dependencies.

### From Source

GEMC uses [Meson](https://mesonbuild.com). A normal source build is:

```shell
git clone --recurse-submodules https://github.com/gemc/src.git gemc-src
cd gemc-src
meson setup build --native-file=core.ini --prefix=/path/to/gemc
meson compile -C build
meson install -C build
```

After installation, add the GEMC binary directory and the bundled Python environment to your shell:

```shell
export GEMC_HOME=/path/to/gemc
export PATH=$GEMC_HOME/bin:$GEMC_HOME/python_env/bin:$PATH
```

For versioned installs such as `/path/to/gemc/0.2`, keep the version in the path:

```shell
export GEMC_VERSION=0.2
export PATH=/path/to/gemc/$GEMC_VERSION/bin:/path/to/gemc/$GEMC_VERSION/python_env/bin:$PATH
```

Verify both the simulator and Python tools:

```shell
gemc -v
gemc-system-template --help
gemc-analyzer --help
```

Build options:

- `-Droot=enabled` enables ROOT output when ROOT is installed.
- `-Di_test=true` enables GUI-oriented tests.
- `meson install -C build` builds and installs the current tree.

### Docker

Pre-built development images are published to GitHub Container Registry. For example:

```shell
docker run -it --rm -v "$PWD":/work ghcr.io/gemc/src:dev-ubuntu-24.04 bash
```

Published image families include:

| OS         | Version                | Architectures     |
|------------|------------------------|-------------------|
| AlmaLinux  | 10    `amd64`, `arm64` |
| Arch Linux | latest                 | `amd64`           |
| Debian     | 13                     | `amd64`, `arm64`  |
| Fedora     | 44                     | `amd64`, `arm64`  |
| Ubuntu     | 24.04, 26.04           | `amd64`, `arm64`  |

See the [installation page](https://gemc.github.io/home/installation/) for Docker, browser GUI, and Apptainer examples.

## Python Geometry Workflow

Detector systems are usually written with [`pygemc`](https://pypi.org/project/pygemc/), the GEMC Python package:

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

### PyVista Visualization

PyVista is part of the normal GEMC geometry workflow. It lets users inspect detector geometry before running
Geant4, export portable VTK.js scenes for documentation, and catch placement or rotation mistakes while editing
Python geometry scripts.
<table>
  <tr>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/b1/b1.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/b1/gemc_view.png"
             alt="B1 PyVista" width="180"><br>
        B1
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/b2/b2.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/b2/gemc_view.png"
             alt="B2 PyVista" width="180"><br>
        B2
      </a>
    </td>
  </tr>
  <tr>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/materials/material.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/materials/gemc_view.png"
             alt="Materials PyVista" width="180"><br>
        Materials
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/scintillator_barrel/scintillator_barrel.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/scintillator_barrel/gemc_view.png"
             alt="Scintillator Barrel PyVista" width="180"><br>
        Scintillator Barrel
      </a>
    </td>
  </tr>
</table>

Open the linked interactive PyVista scenes generated from the GEMC examples.

GitHub README pages cannot embed `.vtksz` scenes directly, so the image above links to the hosted VTK.js viewer.

## Running Simulations

GEMC is configured with YAML files and equivalent command-line options. A compact steering file can define the run number, event count, generator, geometry systems, output streamers, and world volume:

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
    format: json

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

## Output and Analysis

The output layer is plugin-based. Built-in streamer formats include:

| Format    | Notes                                                                                                            |
|-----------|------------------------------------------------------------------------------------------------------------------|
| `ascii`   | Human-readable text output                                                                                       |
| `csv`     | Per-thread CSV tables for digitized hits, true information, generated particles, and tracked generated particles |
| `json`    | Structured event output                                                                                          |
| `jlabsro` | Binary JLAB SRO frame records                                                                                    |
| `root`    | ROOT TTrees, available when GEMC is built with ROOT                                                              |

Analyze CSV or ROOT output with:

```shell
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
gemc-analyzer out.root E --kind root --detector flux --save energy.png
```

## Examples

The repository includes examples under `examples/`, with matching documentation and rendered assets on the project site:

- `examples/basic/simple_flux`: flux digitization and output analysis
- `examples/basic/b1`: Geant4 basic B1-style geometry
- `examples/basic/b2`: Geant4 basic B2-style geometry
- `examples/basic/material`: material and optical-property definitions
- `examples/basic/pyvista`: PyVista visualization examples
- `examples/optical/cherenkov`: optical photon and Cherenkov workflow

## Development

Build and install:

```shell
meson install -C build
```

List tests:

```shell
meson test -C build --list
```

Run one test with logs:

```shell
meson test -C build -v --print-errorlogs <testname>
```

The CI system also builds Docker images, runs the Meson test suite, runs sanitizer builds, generates Doxygen documentation, performs CodeQL analysis, and publishes nightly development release notes.

## Documentation

- [Project homepage](https://gemc.github.io/home/)
- [Installation](https://gemc.github.io/home/installation/)
- [User documentation](https://gemc.github.io/home/documentation/)
- [Examples](https://gemc.github.io/home/examples/)
- [Doxygen workflow](https://github.com/gemc/src/actions/workflows/doxygen.yml)
- [Python geometry API repository](https://github.com/gemc/pygemc)
- [CLAS12 GEMC systems repository](https://github.com/gemc/clas12-systems)
- [Documentation website repository](https://github.com/gemc/home)
- [GEMC2 / CLAS12 repository](https://github.com/gemc/clas12Tags)

This repository is GEMC version 3 and newer. CLAS12 GEMC2 simulations are maintained separately in `gemc/clas12Tags`.

## Contributing

Contributions are welcome through normal pull requests. Before opening a pull request, run the relevant Meson tests and keep changes focused. See:

- [`CONTRIBUTING.md`](CONTRIBUTING.md)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md)
- [`SECURITY.md`](SECURITY.md)

## Citation

If you use GEMC in scientific work, cite:

> M. Ungaro, "Geant4 Monte-Carlo (GEMC) A database-driven simulation program," EPJ Web of Conferences 295, 05005 (2024). https://doi.org/10.1051/epjconf/202429505005

BibTeX and additional citation formats are in [`CITATION.md`](CITATION.md).

## License

GEMC is distributed under the project license in [`LICENSE.md`](LICENSE.md). The software also depends on separately licensed third-party components, including Geant4, CLHEP, Qt, ROOT, SQLite, and Assimp.


[test]: https://github.com/gemc/src/actions/workflows/test.yml
[test-badge]: https://github.com/gemc/src/actions/workflows/test.yml/badge.svg
[deploy]: https://github.com/gemc/src/actions/workflows/deploy.yml
[deploy-badge]: https://github.com/gemc/src/actions/workflows/deploy.yml/badge.svg
[docs]: https://github.com/gemc/src/actions/workflows/doxygen.yml
[docs-badge]: https://github.com/gemc/src/actions/workflows/doxygen.yml/badge.svg
[sanitize]: https://github.com/gemc/src/actions/workflows/sanitize.yml
[sanitize-badge]: https://github.com/gemc/src/actions/workflows/sanitize.yml/badge.svg
[codeql]: https://github.com/gemc/src/actions/workflows/codeql.yml
[codeql-badge]: https://github.com/gemc/src/actions/workflows/codeql.yml/badge.svg
[nightly]: https://github.com/gemc/src/actions/workflows/dev_release.yml
[nightly-badge]: https://github.com/gemc/src/actions/workflows/dev_release.yml/badge.svg
[site]: https://github.com/gemc/home/actions/workflows/jekyll.yml
[site-badge]: https://github.com/gemc/home/actions/workflows/jekyll.yml/badge.svg
[pypi]: https://pypi.org/project/pygemc/
[pypi-badge]: https://img.shields.io/pypi/v/pygemc.svg?cacheSeconds=300
[license]: LICENSE.md
[license-badge]: https://img.shields.io/badge/license-GEMC-blue.svg
