# GEMC — Geant4 Monte-Carlo simulation framework

**GEMC** (the *GEant Monte-Carlo*) is a database-driven [Geant4](https://geant4.web.cern.ch) application for
detector and radiation-transport simulations. Detector descriptions live outside the compiled C++ application:
you define geometry, materials, optics, sensitive detectors, and run configuration in Python or data files, and
GEMC loads them at run time to execute the full Geant4 pipeline.

These images ship a ready-to-run GEMC development environment — the compiled `gemc` simulator, the bundled
`pygemc` Python tools, and all Geant4 dependencies — so you can simulate without managing a local Geant4 build.

- 🏠 Homepage & docs: https://gemc.github.io/home/
- 📦 Source: https://github.com/gemc/src
- 🐍 Python API (`pygemc`): https://pypi.org/project/pygemc/

> These images are mirrored from the canonical GitHub Container Registry build at `ghcr.io/gemc/src`.
> The two registries carry identical content and tags; use whichever is closer to your infrastructure.

<br/>

## Quick start

```shell
docker run -it --rm -v "$PWD":/work docker.io/gemc/gemc:dev-ubuntu-24.04 bash
```

Inside the container the `gemc` binary and the `pygemc` Python environment are already on the `PATH`:

```shell
gemc -v                         # version sanity check
gemc-system-template -s counter # scaffold a minimal detector system
cd counter && ./counter.py      # build geometry (writes gemc.db)
gemc counter.yaml               # run the simulation
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
```

<br/>

## Supported tags

Tags follow the pattern `<gemc-version>-<os>-<os-version>` and are multi-architecture
(`linux/amd64` + `linux/arm64`, except Arch Linux which is `amd64`-only).

| OS         | Example tag          | Architectures    |
|------------|----------------------|------------------|
| AlmaLinux  | `dev-almalinux-10`   | `amd64`, `arm64` |
| Arch Linux | `dev-archlinux-latest` | `amd64`        |
| Debian     | `dev-debian-13`      | `amd64`, `arm64` |
| Fedora     | `dev-fedora-44`      | `amd64`, `arm64` |
| Ubuntu     | `dev-ubuntu-24.04`   | `amd64`, `arm64` |
| Ubuntu     | `dev-ubuntu-26.04`   | `amd64`, `arm64` |

The `dev` prefix tracks the latest `main`; versioned release tags follow the same scheme.

<br/>

## What's inside

- The compiled `gemc` Geant4 simulator
- The bundled `pygemc` Python environment (geometry building, PyVista preview, VTK.js export, analyzer)
- Geant4, CLHEP, Xerces-C, and the supporting toolchain

Key GEMC features:

- Python-first detector definition through `pygemc`
- Geometry/material storage in SQLite or GEMC ASCII databases; imports from GDML and CAD meshes
- Built-in sensitive-detector digitizations: `flux`, `gPhotonDetector`, `dosimeter`, `particle_counter`
- C++ plugin infrastructure for custom digitization, fields, particle readers, and output streamers
- Output streamers for ASCII, CSV, JSON, JLAB SRO, and optional ROOT
- Magnetic-field support, including a built-in multipole field plugin
- Interactive Geant4/Qt visualization and off-screen image generation

<br/>

## Mounting your work

The examples above mount the current directory at `/work`. Use a bind mount to keep geometry scripts, steering
cards, and output on the host:

```shell
docker run -it --rm -v "$PWD":/work -w /work docker.io/gemc/gemc:dev-ubuntu-24.04 bash
```

For the interactive Geant4/Qt GUI and Apptainer recipes, see the
[installation page](https://gemc.github.io/home/installation/).

<br/>

## Documentation & support

- [Installation guide](https://gemc.github.io/home/installation/)
- [Quickstart tutorial](https://gemc.github.io/home/documentation/quickstart/)
- [Examples gallery](https://gemc.github.io/home/examples/)
- [Issue tracker](https://github.com/gemc/src/issues)

<br/>

## Citation

If you use GEMC in scientific work, please cite:

> M. Ungaro, "Geant4 Monte-Carlo (GEMC) A database-driven simulation program," EPJ Web of Conferences 295,
> 05005 (2024). https://doi.org/10.1051/epjconf/202429505005

## License

GEMC is licensed under the [Apache License, Version 2.0](https://github.com/gemc/src/blob/main/LICENSE).
The images also contain separately licensed third-party components, including Geant4, CLHEP, Qt, ROOT, SQLite, and
Assimp.
