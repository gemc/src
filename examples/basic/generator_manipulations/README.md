# Generator manipulations

Upcoming in the next release.

Each YAML isolates one particle vertex or angular option using the same geometry: a liquid hydrogen tube of
radius 25 mm and length 40 mm, centered at the origin inside a vacuum world. A 1 GeV geantino starts inside
the target, representing a particle produced in a target event. Geantinos neither scatter nor create
secondaries,
so the flux-sensitive target records one primary hit per throw. `recordZeroEdep: true` retains those
noninteracting tracks.

## Cases

All coordinates below are in mm. Uniform spreads are half-widths; Gaussian spreads are standard deviations.
The smearing cards use the displaced center (2, -3, 0) to demonstrate that random offsets are added to the
nominal position. Z smearing uses theta = 90 degrees; XY and sphere smearing use theta = 45 degrees.
All vertex-smearing cards use phi = 90 degrees.
The target is shown at 20% opacity so the trajectories remain visible through it.

| YAML | Expected thrown vertex |
| --- | --- |
| `uniform_xy.yaml` | Independent x on [0, 4] and y on [-6, 0]; z = 0. |
| `uniform_z.yaml` | z uniform on [-20, 20]; x = 2, y = -3. |
| `gaussian_xy.yaml` | Independent x/y Gaussians with means (2, -3), sigmas (4, 5); z = 0. |
| `gaussian_z.yaml` | z Gaussian with mean 0 and sigma 3; x = 2, y = -3. |
| `sphere.yaml` | Uniform inside a sphere centered at (2, -3, 0), radius 3. |

For `randomVertexModel: sphere`, the radius is
`sqrt(delta_vx^2 + delta_vy^2 + delta_vz^2)`: the supplied spreads (1, 2, 2) give radius 3 mm.
They do not define three ellipsoid axes. The one-dimensional projections are parabolic, and the radial
cumulative distribution is `P(r < R*u) = u^3`.

The angular cards keep the vertex fixed at the target center, (0, 0, 0) mm.

| YAML | Expected thrown angles (degrees) |
| --- | --- |
| `fixed_theta_and_phi.yaml` | theta = 3, phi = 45. |
| `uniform_theta.yaml` | theta uniform on [-18, 22], phi = 0. |
| `gaussian_theta.yaml` | theta Gaussian with mean 2 and sigma 20, phi = 0. |
| `cosine_theta.yaml` | cos(theta) uniform between cos(22 degrees) and cos(0 degrees), phi = 0. |
| `uniform_phi.yaml` | phi uniform on [45, 135], theta = 45. |
| `phi_wraparound.yaml` | phi uniform on [330, 370], crossing 360 degrees, theta = 45. |

`randomThetaModel` accepts `uniform`, `gaussian`, and `cosine`. For the cosine model, `delta_theta`
sets the angular half-width and the probability density is proportional to sin(theta) within that interval.
The three theta-smearing cards use `delta_theta: 20*deg` with nominal theta = 2 degrees. Uniform and Gaussian
sampling allow signed theta; a negative theta reverses the transverse momentum. The reconstructed polar
angle is nonnegative, with phi shifted by 180 degrees for negative theta. Cosine sampling intersects the
requested interval with [0, 180] degrees, giving [0, 22] degrees here.
Phi always uses uniform sampling with half-width `delta_phi`; GEMC has no `randomPhiModel` option.

## Run and view the plots

From this example directory, with GEMC and pygemc installed:

```sh
python generator_manipulations.py -f ascii
gemc uniform_z.yaml -gui
```

Generate events using the GUI's beam-on control, then open the Analyzer. The combined XY cards each prepare
one two-dimensional `vy`-versus-`vx` plot. The Z and sphere cards prepare histograms of `vx`, `vy`, and `vz`,
plus a two-dimensional vertex projection. The titles state the expected fixed value,
half-width, sigma, or sphere radius. Accumulate is enabled so repeated beam-on calls improve the statistics.
The prepared GUI plots require a GEMC build with Qt Charts.

The Analyzer uses `plugin: flux`, `source: "true"`, and the original track vertex fields, in mm.
These are the positions at which particles were thrown; the hit-position fields `avgx/avgy/avgz` describe
transport through the target. The noninteracting single-particle beam makes the vertex plots unbiased.
Gaussian plot windows extend to five sigma; these display limits do not truncate the generator.
Gaussian Z uses sigma = 3 mm, with more than six sigma inside the 20 mm target half-length. Gaussian tails
are unbounded; every vertex in the seeded example samples is checked to lie inside the target.

The angular cards prepare histograms of `px`, `py`, and `pz`. Gaussian and cosine theta use a `px`-versus-`pz`
projection (horizontal `pz`, vertical `px`); the other cards use `py` versus `px`.
The Analyzer exposes these true-information fields directly, in MeV/c. Their expected values follow
`px = p*sin(theta)*cos(phi)`, `py = p*sin(theta)*sin(phi)`, and `pz = p*cos(theta)`.
The cosine-theta case therefore has a flat pz distribution; fixed theta with a phi spread gives an arc in
the px/py projection. Geantino transport preserves these momenta.

Run any card without `-gui` for a seeded 5,000-event batch sample. Its CSV output prefix matches the card name,
so cases have separate output files. Built-in `ganalysis` plots are displayed in the GUI, not exported by batch
runs.

The [website guide](https://gemc.github.io/home/examples/basic/generator_manipulations)
shows a 100-event GEMC view and a verified analysis plot for every case.

## Automated verification

From the repository root, run the configured Meson suite:

```sh
meson test -C build --suite generator_manipulations --print-errorlogs
```

Each test generates fresh geometry and runs its card in a temporary directory, so it also works in isolation.
The checks require every generated vertex to be inside the target and exactly one target hit per particle.
They match each hit's vertex to its generated record by event ID and validate every configured Analyzer
variable against the true-information CSV.
They check fixed coordinates, uniform bounds and shape, Gaussian shape, and the sphere's radius, projections,
and volume distribution using a conservative cumulative-distribution tolerance. The combined XY cases also
check that the sampled transverse coordinates are uncorrelated.
Angular checks also compare the momentum components with the generated records, reconstruct theta and phi
(modulo 360 degrees), and check the fixed, uniform, Gaussian, and cosine angular distributions.

To check one card directly with the build-tree environment:

```sh
build/subprojects/pygemc/python_env/bin/python \
  examples/basic/generator_manipulations/check_generator.py build/bin/gemc \
  examples/basic/generator_manipulations/uniform_z.yaml
```
