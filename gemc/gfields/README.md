# gfields

The Geant4 examples usually define a `FieldSetup` class containing:

```
G4FieldManager*          fFieldManager = nullptr;
G4ChordFinder*           fChordFinder = nullptr;
G4Mag_UsualEqRhs*        fEquation = nullptr;
G4MagneticField*         fMagneticField = nullptr;

G4MagIntegratorStepper*  fStepper = nullptr;
G4bool                   fUseFSALstepper = false;
G4VIntegrationDriver*    fDriver =  nullptr;  // If non-null, its new type (FSAL)
G4int                    fStepperType = -1;
```

However, it is not necessary to keep all these variables in the class. The custom class can be
derived from `G4FieldManager`, with:

- `G4Mag_UsualEqRhs`, `G4MagIntegratorStepper`, `G4ChordFinder`, and `G4FieldManager` defined in `create_FieldManager`
- `GetFieldValue` as the pure virtual method implemented by the various factories

## Field plugins

Fields are implemented as plugins (shared libraries exposing the C symbol `GFieldFactory`) selected by
the `type` key of a `-gfields` entry, which resolves to the library `gfield<type>Factory`. The in-tree
plugins live under `gfieldFactories/`:

- `multipoles` (`type: multipoles`): ideal analytic multipole / solenoid field.
- `asciimap` (`type: asciimap`): generic ASCII field map. It is the GEMC3 successor of the clas12Tags
  `asciiField`, with the two responsibilities split apart:
  - the **definition** (symmetry, grid coordinates, units, interpolation, placement) is given in YAML
    through the `-gfields` node, like the clas12-systems plugins;
  - the **map file** holds only data rows — the coordinate columns (in `coordinate1`, `coordinate2`,
    `coordinate3` order) followed by the field components — with no embedded XML header.

### asciimap data format and definition

Supported symmetries and their coordinate names:

| symmetry                              | coordinates                            | field columns |
|---------------------------------------|----------------------------------------|---------------|
| `dipole-x` / `dipole-y` / `dipole-z`  | `longitudinal`, `transverse`           | 1 (on-axis)   |
| `cylindrical-x/y/z`                   | `transverse`, `longitudinal`           | 2 (BT, BL)    |
| `phi-segmented`                       | `azimuthal`, `transverse`, `longitudinal` | 3 (Bx,By,Bz) |
| `cartesian_3D` / `cartesian_3D_quadrant` | `X`, `Y`, `Z`                       | 3 (Bx,By,Bz)  |

Each grid axis is one scalar `coordinate<n>: "name, npoints, min, max"`; `min`/`max` are Geant4-number
expressions whose unit also sets the unit of the matching coordinate column in the map file. Map rows
may appear in any order — each row's grid index is computed from its coordinate columns and validated
against the grid. Field values are read in `field_unit` (default `gauss`) and multiplied by `scale`
(default `1`). Optional `vx,vy,vz` shift the query point and `rx,ry,rz` rotate the field.

Example (the in-tree example under `examples/`):

```yaml
gfields:
  - name: dipole
    type: asciimap
    symmetry: dipole-z
    map: dipole_map.txt
    field_unit: T
    coordinate1: "longitudinal, 3, 0*cm, 2*cm"
    coordinate2: "transverse,   2, 0*cm, 1*cm"
```

Compared with the legacy `asciiField`, the values are stored in contiguous buffers addressed with
precomputed strides (instead of `float**`/`float***`), the symmetry is decoded once into an enum so the
hot `GetFieldValue` loop does not compare strings, and the rotation trigonometry is cached at load time.

### Migrating a legacy map

`examples/solenoid.yaml` (cylindrical-z) and `examples/torus.yaml` (phi-segmented) show
one-to-one translations of legacy clas12 `<mfield>` headers. The XML `<symmetry type=...>` becomes
`symmetry`, each `<first>/<second>/<third>` coordinate becomes a `coordinate<n>: "name, npoints, min,
max"` line (the `units` attribute moves into the min/max expression), `<field unit=...>` becomes
`field_unit`, and `<interpolation type=.../>` becomes `interpolation`. The symmetry attributes
`integration`/`minStep` map to the standard `integration_stepper`/`minimum_step` keys. The map file
itself loses the embedded XML header and keeps only its data rows (see `examples/solenoid_map.txt` and
`examples/torus_map.txt`). The example grids are shrunk to small complete maps so they run as tests; the
real clas12 maps are 601 x 1201 (solenoid) and 16 x 2501 x 251 (torus).
