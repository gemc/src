# Vertex and angular manipulation

Upcoming in the next release.

Each YAML isolates one particle vertex or angular option using the same geometry: a liquid hydrogen tube of
radius 25 mm and length 40 mm, centered at the origin inside a vacuum world. A 1 GeV geantino travels near +z
from upstream of the target. Geantinos neither scatter nor create secondaries, so the flux-sensitive target
records one primary hit per throw. `recordZeroEdep: true` retains those noninteracting tracks.

## Cases

All coordinates below are in mm. Uniform spreads are half-widths; Gaussian spreads are standard deviations.
The three positioning cards have zero spread. The smearing cards use the displaced center (2, -3, -300)
to demonstrate that random offsets are added to the nominal position.

| YAML | Expected thrown vertex |
| --- | --- |
| `position_x.yaml` | Fixed at (2, 0, -300). |
| `position_y.yaml` | Fixed at (0, -3, -300). |
| `position_z.yaml` | Fixed at (0, 0, -250). |
| `uniform_x.yaml` | x uniform on [0, 4]; y = -3, z = -300. |
| `uniform_y.yaml` | y uniform on [-6, 0]; x = 2, z = -300. |
| `uniform_z.yaml` | z uniform on [-320, -280]; x = 2, y = -3. |
| `gaussian_x.yaml` | x Gaussian with mean 2 and sigma 2; y = -3, z = -300. |
| `gaussian_y.yaml` | y Gaussian with mean -3 and sigma 3; x = 2, z = -300. |
| `gaussian_z.yaml` | z Gaussian with mean -300 and sigma 20; x = 2, y = -3. |
| `sphere.yaml` | Uniform inside a sphere centered at (2, -3, -300), radius 3. |

For `randomVertexModel: sphere`, the radius is
`sqrt(delta_vx^2 + delta_vy^2 + delta_vz^2)`: the supplied spreads (1, 2, 2) give radius 3 mm.
They do not define three ellipsoid axes. The one-dimensional projections are parabolic, and the radial
cumulative distribution is `P(r < R*u) = u^3`.

The angular cards keep the vertex fixed at (0, 0, -300) mm. Their small forward angles keep the beam within
the target acceptance.

| YAML | Expected thrown angles (degrees) |
| --- | --- |
| `theta_position.yaml` | theta = 3, phi = 0. |
| `phi_position.yaml` | theta = 3, phi = 45. |
| `uniform_theta.yaml` | theta uniform on [1, 3], phi = 0. |
| `gaussian_theta.yaml` | theta Gaussian with mean 2 and sigma 0.3, phi = 0. |
| `cosine_theta.yaml` | cos(theta) uniform between cos(3 degrees) and cos(1 degree), phi = 0. |
| `uniform_phi.yaml` | phi uniform on [45, 135], theta = 3. |
| `phi_wraparound.yaml` | phi uniform on [330, 370], crossing 360 degrees, theta = 3. |

`randomThetaModel` accepts `uniform`, `gaussian`, and `cosine`. For the cosine model, `delta_theta`
sets the angular half-width and the probability density is proportional to sin(theta) within that interval.
Phi always uses uniform sampling with half-width `delta_phi`; GEMC has no `randomPhiModel` option.

## Run and view the plots

From this example directory, with GEMC and pygemc installed:

```sh
python vertex_manipulation.py -f ascii
gemc uniform_z.yaml -gui
```

Generate events using the GUI's beam-on control, then open the Analyzer. Each vertex card prepares histograms of
`vx`, `vy`, and `vz`, plus a two-dimensional vertex projection. The titles state the expected fixed value,
half-width, sigma, or sphere radius. Accumulate is enabled so repeated beam-on calls improve the statistics.
The prepared GUI plots require a GEMC build with Qt Charts.

The Analyzer uses `plugin: flux`, `source: "true"`, and the original track vertex fields, in mm.
These are the positions at which particles were thrown; the hit-position fields `avgx/avgy/avgz` describe
transport through the target. The noninteracting single-particle beam makes the vertex plots unbiased.
Gaussian plot windows extend to five sigma; these display limits do not truncate the generator.

The angular cards prepare histograms of `px`, `py`, and `pz`, plus a transverse momentum projection.
The Analyzer exposes these true-information fields directly, in MeV/c. Their expected values follow
`px = p*sin(theta)*cos(phi)`, `py = p*sin(theta)*sin(phi)`, and `pz = p*cos(theta)`.
The cosine-theta case therefore has a flat pz distribution; fixed theta with a phi spread gives an arc in
the px/py projection. Geantino transport preserves these momenta.

Run any card without `-gui` for a seeded 5,000-event batch sample. Its CSV output prefix matches the card name,
so cases have separate output files. Built-in `ganalysis` plots are displayed in the GUI, not exported by batch
runs.

## Automated verification

From the repository root, run the configured Meson suite:

```sh
meson test -C build --suite vertex_manipulation --print-errorlogs
```

Each test generates fresh geometry and runs its card in a temporary directory, so it also works in isolation.
The checks require exactly one target hit per generated particle, match each hit's vertex to its generated
record by event ID, and validate every configured Analyzer variable against the true-information CSV.
They check fixed coordinates, uniform bounds and shape, Gaussian shape, and the sphere's radius, projections,
and volume distribution using a conservative cumulative-distribution tolerance.
Angular checks also compare the momentum components with the generated records, reconstruct theta and phi
(modulo 360 degrees), and check the fixed, uniform, Gaussian, and cosine angular distributions.

To check one card directly with the build-tree environment:

```sh
build/subprojects/pygemc/python_env/bin/python \
  examples/basic/vertex_manipulation/check_vertex.py build/bin/gemc \
  examples/basic/vertex_manipulation/uniform_z.yaml
```
