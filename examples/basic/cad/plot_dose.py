#!/usr/bin/env python3
"""Plot the absorbed dose in each organ for the CAD brachytherapy example.

The example fires an isotropic Iridium-192 photon source from inside the liver (see cad.yaml) and
records, for every organ, one "flux" hit per energy deposit (organ identity + totEdep). This script:

  1. sums the deposited energy per organ from  <root>_t0_digitized.csv,
  2. divides by the organ mass (CAD mesh volume x material density) to get the absorbed dose,
  3. derives the real Ir-192 irradiation time that the simulated photon count represents, and
  4. draws a bar chart of dose per organ with that time in the title.

Run it (from this directory) after producing the CSV, e.g.:

    /opt/projects/gemc/src/build/bin/gemc cad.yaml -n=200 -gstreamer="[{format: csv, filename: organs}]"
    /opt/projects/gemc/src/build/subprojects/pygemc/python_env/bin/python plot_dose.py

Options: plot_dose.py [root] [--activity CI] [--yield Y] [--save PATH]
  root        CSV rootname written by the gstreamer (default: organs)
  --activity  Ir-192 source activity in curie (default: 10 Ci, a nominal HDR afterloader source)
  --yield     photons emitted per Ir-192 decay (default: 2.3, the summed gamma-line intensity)
"""

import argparse
import os
import struct
import sys

HERE = os.path.dirname(os.path.abspath(__file__))

# Physical constants.
MEV_TO_JOULE = 1.602176634e-13   # J per MeV
CURIE_TO_BQ = 3.7e10             # decays per second per curie

# Organs, keyed by the "organ" identifier written into the CSV. Each carries the mesh file, the
# uniform CAD scale, and the assigned material density (g/cm^3, as reported by
# `gemc cad.yaml -printSystemsMaterials`). These must match stls/cad__default.yaml.
ORGANS = {
    1: {"name": "heart", "stl": "heart_NIH3D.stl",       "scale": 130.0,   "density": 1.06, "color": "#c81e1e"},
    2: {"name": "lungs", "stl": "respiratory_NIH3D.stl", "scale": 9.817,   "density": 1.04, "color": "#f2a0a0"},
    3: {"name": "liver", "stl": "liver_NIH3D.stl",       "scale": 640.9,   "density": 1.03, "color": "#8b5a2b"},
}


def mesh_volume_mm3(path, scale):
    """Signed volume of a binary STL (native units), scaled to mm^3 by scale**3."""
    with open(path, "rb") as f:
        f.read(80)
        n = struct.unpack("<I", f.read(4))[0]
        vol = 0.0
        for _ in range(n):
            d = f.read(50)
            a = struct.unpack("<3f", d[12:24])
            b = struct.unpack("<3f", d[24:36])
            c = struct.unpack("<3f", d[36:48])
            vol += (a[0] * (b[1] * c[2] - c[1] * b[2])
                    - a[1] * (b[0] * c[2] - c[0] * b[2])
                    + a[2] * (b[0] * c[1] - c[0] * b[1])) / 6.0
    return abs(vol) * scale ** 3


def organ_mass_kg(organ):
    """Organ mass (kg) = density * mesh volume."""
    vol_cm3 = mesh_volume_mm3(os.path.join(HERE, "stls", organ["stl"]), organ["scale"]) / 1000.0
    return vol_cm3 * organ["density"] / 1000.0   # g/cm^3 * cm^3 -> g -> kg


def sum_edep_per_organ(digitized_csv):
    """Total deposited energy (MeV) per organ id, read from the flux digitized stream."""
    edep = {oid: 0.0 for oid in ORGANS}
    with open(digitized_csv) as f:
        header = [c.strip() for c in f.readline().split(",")]
        i_organ = header.index("organ")
        i_edep = header.index("totEdep")
        for line in f:
            cols = line.split(",")
            if len(cols) <= max(i_organ, i_edep):
                continue
            oid = int(float(cols[i_organ]))
            if oid in edep:
                edep[oid] += float(cols[i_edep])
    return edep


def count_primaries(generated_csv):
    """Number of primary photons simulated = rows in the generated stream."""
    with open(generated_csv) as f:
        return sum(1 for _ in f) - 1   # minus the header row


def human_time(seconds):
    """Format a duration with a sensible unit."""
    for unit, factor in (("s", 1.0), ("ms", 1e-3), ("us", 1e-6), ("ns", 1e-9)):
        if seconds >= factor:
            return f"{seconds / factor:.3g} {unit}"
    return f"{seconds / 1e-12:.3g} ps"


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("root", nargs="?", default="organs", help="CSV rootname (default: organs)")
    parser.add_argument("--activity", type=float, default=10.0, help="Ir-192 activity in Ci (default: 10)")
    parser.add_argument("--yield", dest="photon_yield", type=float, default=2.3,
                        help="photons per Ir-192 decay (default: 2.3)")
    parser.add_argument("--save", help="write the figure to this path instead of showing it")
    args = parser.parse_args(argv)

    digitized_csv = os.path.join(HERE, f"{args.root}_t0_digitized.csv")
    generated_csv = os.path.join(HERE, f"{args.root}_t0_generated.csv")
    if not os.path.exists(digitized_csv):
        parser.error(f"{digitized_csv} not found - run gemc cad.yaml first (see the module docstring).")

    edep = sum_edep_per_organ(digitized_csv)
    n_gamma = count_primaries(generated_csv) if os.path.exists(generated_csv) else None

    # Absorbed dose per organ: D = edep / mass.
    order = [3, 1, 2]   # liver (target), heart, lungs
    names, doses, colors = [], [], []
    print(f"{'organ':8s} {'edep [MeV]':>12s} {'mass [g]':>10s} {'dose [Gy]':>12s}")
    for oid in order:
        organ = ORGANS[oid]
        mass = organ_mass_kg(organ)
        dose = edep[oid] * MEV_TO_JOULE / mass   # Gy
        names.append(organ["name"])
        doses.append(dose)
        colors.append(organ["color"])
        print(f"{organ['name']:8s} {edep[oid]:12.4g} {mass * 1000:10.1f} {dose:12.4e}")

    # Irradiation time the simulated photon count represents:
    #   emission rate R = A * Y  (photons/s);  t_rad = N_gamma / R.
    activity_bq = args.activity * CURIE_TO_BQ
    rate = activity_bq * args.photon_yield
    t_rad = n_gamma / rate if n_gamma else None

    import matplotlib
    if args.save:
        matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    fig, ax = plt.subplots(figsize=(7, 4.5))
    bars = ax.bar(names, doses, color=colors, edgecolor="black")
    ax.set_yscale("log")
    ax.set_ylabel("absorbed dose  [Gy]")
    ax.set_xlabel("organ")
    for bar, dose in zip(bars, doses):
        ax.annotate(f"{dose:.2e} Gy", (bar.get_x() + bar.get_width() / 2, dose),
                    ha="center", va="bottom", fontsize=8)

    title = "Ir-192 brachytherapy source in the liver — dose per organ"
    if t_rad is not None:
        title += (f"\n{args.activity:g} Ci source, {n_gamma:,} photons "
                  f"→ irradiation time = {human_time(t_rad)}")
    ax.set_title(title, fontsize=10)
    ax.margins(y=0.25)
    fig.tight_layout()

    if args.save:
        fig.savefig(args.save, bbox_inches="tight", dpi=130)
        print(f"saved {args.save}")
    else:
        plt.show()


if __name__ == "__main__":
    sys.exit(main())
