# GEMC: Geant4 Monte-Carlo 

### Pull requests

[![Almalinux Build](https://github.com/gemc/src/actions/workflows/build_gemc_almalinux.yml/badge.svg)](https://github.com/gemc/src/actions/workflows/build_gemc_almalinux.yml)
[![Fedora Build](https://github.com/gemc/src/actions/workflows/build_gemc_fedora.yml/badge.svg)](https://github.com/gemc/src/actions/workflows/build_gemc_fedora.yml)
[![Ubuntu Build](https://github.com/gemc/src/actions/workflows/build_gemc_ubuntu.yml/badge.svg)](https://github.com/gemc/src/actions/workflows/build_gemc_ubuntu.yml)
[![Sanitize](https://github.com/gemc/src/actions/workflows/sanitize.yaml/badge.svg)](https://github.com/gemc/src/actions/workflows/sanitize.yaml)

### Nightly

[![Nightly Dev Release](https://github.com/gemc/src/actions/workflows/dev_release.yml/badge.svg)](https://github.com/gemc/src/actions/workflows/dev_release.yml)
[![Doxygen](https://github.com/gemc/src/actions/workflows/doxygen.yaml/badge.svg)](https://github.com/gemc/src/actions/workflows/doxygen.yaml)



## Installing GEMC with meson


## Sanitizers

- none, address, thread, undefined, memory, leak (meson configure)

Working on mac:

- undefined

## Validation

The validation includes the following workflows

- Compile and install gemc
- Run gemc modules tests
- Run API tests


## Multithreading

### Shared classes: geometry and physics tables are shared:
- G4VUserDetectorConstruction,
- G4VUserPhysicsList
- G4VUserActionInitialization
- Pretty sure GRun

### Local thread classes:
- EventManager
- TrackingManager
- SteppingManager
- TransportationManager
- GeometryManager
- FieldManager
- Navigator
- SensitiveDetectorManager

# Validation



### Known issues:

- on ubuntuu  somehow libz is linked statically, and the compilation fails. Somehow using sanitize 'undefined' fixes this.