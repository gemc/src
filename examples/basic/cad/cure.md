# Curing the organ meshes

The STL files in `stls/` are **cured** versions of the raw NIH 3D scans kept in `stls/source/`. The raw
scans are heavy (hundreds of thousands of triangles) and not watertight, so Geant4 loads them slowly and
prints `G4TessellatedSolid` warnings ("holes", "wrong orientation", "negative cubic volume").

They were simplified and repaired with the pygemc [`gemc-cure-mesh`](https://github.com/gemc/pygemc)
utility. Each organ uses the lightest recipe that cleans it up (see
[Cure Mesh](https://gemc.github.io/home/documentation/api/cure_mesh) for the options):

```sh
# heart: the default weld / repair / decimate pass cleans it up to a watertight solid
gemc-cure-mesh stls/source/heart_NIH3D.stl       -o stls/heart_NIH3D.stl       -f 15000

# lungs: two thin-walled lobes; the default pass is the best that is stable here (Poisson and
# self-intersection removal abort on this mesh)
gemc-cure-mesh stls/source/respiratory_NIH3D.stl -o stls/respiratory_NIH3D.stl -f 25000

# liver: fragmented into dozens of shells; rebuild one watertight manifold with Poisson
gemc-cure-mesh stls/source/liver_NIH3D.stl       -o stls/liver_NIH3D.stl       -f 15000 \
    --reconstruct poisson --remove-self-intersections
```

Run each command in its own process (that is how `gemc-cure-mesh` is meant to be used): pymeshlab's native
engine is not reliable when several meshes are handled back-to-back in one interpreter. The commands read
from `stls/source/` and overwrite `stls/`, so they are safe to re-run.

## Before and after

| Mesh  | Facets            | File size        | Result                                    |
|-------|-------------------|------------------|-------------------------------------------|
| heart | 149,992 → 14,990  | 7.5 MB → 0.75 MB | watertight; holes and self-intersections cleared |
| lungs | 392,402 → 24,933  | 19.6 MB → 1.25 MB | one small hole left; much fewer defects   |
| liver | 60,369 → 14,998   | 3.0 MB → 0.75 MB | rebuilt as a single watertight manifold   |

After curing, the whole system loads in about a second (versus ~24 s for the raw scans) and the many
tessellated-solid warnings drop to at most one "holes" plus one "orientation" line. The heart and liver are
watertight; the lungs keep a single tiny boundary hole from the source scan (a harmless "holes" warning)
that the automated tools could not close without destabilizing the mesh.

## Lung orientation fix

Geant4's `G4TessellatedSolid` decides *inside vs. outside* from the facet winding. The cured lung mesh came
out of `gemc-cure-mesh` with its facets wound the wrong way (a **negative** signed volume), so Geant4 treated
the lung interior as *outside* the solid and no energy was ever scored there — the organ was invisible to the
dosimetry. The shipped `respiratory_NIH3D.stl` has therefore been re-exported with every facet reversed
(vertex order swapped and normal negated), giving a positive signed volume so the lungs score dose correctly.
The heart and liver already had the correct orientation and were left untouched. The single boundary hole
remains (harmless), but the inside/outside test is now well defined.
