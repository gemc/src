#!/usr/bin/env python3
"""Run one steering card in isolation and check the original vertices used by its Analyzer plots."""

import csv
import math
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

import yaml


def run(command, directory):
    result = subprocess.run(command, cwd=directory, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=40)
    if result.returncode:
        raise RuntimeError(f"Command failed ({result.returncode}): {command}\n{result.stdout[-6000:]}")


def read_rows(path):
    with path.open(newline="") as stream:
        return list(csv.DictReader(stream, skipinitialspace=True))


def millimeters(value):
    number, unit = value.split("*")
    return float(number) * {"mm": 1, "cm": 10}[unit]


def check_cdf(values, cdf):
    """A conservative Kolmogorov distance bound for 5,000 seeded throws."""
    count = len(values)
    distance = max(max(abs(cdf(value) - i / count), abs(cdf(value) - (i + 1) / count))
                   for i, value in enumerate(sorted(values)))
    assert distance < 0.04, f"Distribution disagrees with expectation: D = {distance:.4f}"


def check_vertices(rows, particle):
    center = [millimeters(particle["v" + axis]) for axis in "xyz"]
    delta = [millimeters(particle["delta_v" + axis]) for axis in "xyz"]
    model = particle["randomVertexModel"]
    radius = math.sqrt(sum(value * value for value in delta))
    offsets = [[float(row["v" + axis]) - center[i] for i, axis in enumerate("xyz")] for row in rows]
    assert all(math.isfinite(value) for point in offsets for value in point)
    # CSV uses six significant digits; allow its rounding at the -300 mm longitudinal offset.
    tolerance = 0.001
    for i, axis in enumerate("xyz"):
        values = [point[i] for point in offsets]
        if model == "sphere":
            assert max(abs(value) for value in values) <= radius + tolerance
            check_cdf(values, lambda value: 0.5 + 0.75 * (value / radius)
                      - 0.25 * (value / radius) ** 3)
        elif delta[i] == 0:
            assert all(abs(value) < tolerance for value in values), f"Fixed v{axis} moved"
        elif model == "uniform":
            assert max(abs(value) for value in values) <= delta[i] + tolerance
            check_cdf(values, lambda value: (value / delta[i] + 1) / 2)
        elif model == "gaussian":
            check_cdf(values, lambda value: (1 + math.erf(value / (delta[i] * math.sqrt(2)))) / 2)
        else:
            raise AssertionError(f"Unexpected vertex model: {model}")
    if model == "sphere":
        radii = [math.sqrt(sum(value * value for value in point)) for point in offsets]
        assert max(radii) <= radius + tolerance
        # Uniform in volume has P(r < R*u) = u^3, not a uniform radius or a spherical shell.
        check_cdf(radii, lambda value: (value / radius) ** 3)


def check_angles(rows, particle):
    for axis in ("theta", "phi"):
        center = float(particle[axis].removesuffix("*deg"))
        delta = float(particle.get("delta_" + axis, "0*deg").removesuffix("*deg"))
        # Generated banks use Geant4 internal angular units (radians).
        values = [math.degrees(float(row[axis])) for row in rows]
        model = particle.get("randomThetaModel", "uniform") if axis == "theta" else "uniform"
        if delta == 0:
            assert all(abs(value - center) < 0.001 for value in values), f"Fixed {axis} moved"
        elif model == "gaussian":
            check_cdf(values, lambda value: (1 + math.erf((value - center) / (delta * math.sqrt(2)))) / 2)
        else:
            low, high = center - delta, center + delta
            assert min(values) >= low - 0.001 and max(values) <= high + 0.001
            if model == "uniform":
                check_cdf(values, lambda value: (value - low) / (high - low))
            elif model == "cosine":
                cos_low, cos_high = math.cos(math.radians(low)), math.cos(math.radians(high))
                check_cdf(values, lambda value: (cos_low - math.cos(math.radians(value)))
                          / (cos_low - cos_high))
            else:
                raise AssertionError(f"Unexpected angular model: {model}")


def check_momentum(hit, thrown):
    theta, phi = (float(thrown[angle]) for angle in ("theta", "phi"))
    momentum = float(thrown["p"])
    expected = (momentum * math.sin(theta) * math.cos(phi),
                momentum * math.sin(theta) * math.sin(phi), momentum * math.cos(theta))
    actual = [float(hit["p" + axis]) for axis in "xyz"]
    for component, reference in zip(actual, expected):
        assert math.isclose(component, reference, abs_tol=0.002), "Analyzer momentum disagrees with throw"
    measured_theta = math.degrees(math.atan2(math.hypot(*actual[:2]), actual[2]))
    assert abs(measured_theta - math.degrees(theta)) < 0.002
    if math.hypot(*actual[:2]) > 0.001:
        measured_phi = math.degrees(math.atan2(actual[1], actual[0]))
        difference = (measured_phi - math.degrees(phi) + 180) % 360 - 180
        assert abs(difference) < 0.002, "Reconstructed phi disagrees with throw modulo 360 degrees"


def main():
    gemc, card = (Path(arg).resolve() for arg in sys.argv[1:])
    config = yaml.safe_load(card.read_text())
    with tempfile.TemporaryDirectory(prefix=f"gemc-vertex-{card.stem}-") as temporary:
        directory = Path(temporary)
        shutil.copyfile(card, directory / card.name)
        run([sys.executable, str(card.parent / "vertex_manipulation.py"), "-f", "ascii"], directory)
        run([str(gemc), card.name], directory)
        base = config["gstreamer"][0]["filename"]
        generated_files = list(directory.glob(base + "*_generated.csv"))
        true_files = list(directory.glob(base + "*_true_info.csv"))
        assert len(generated_files) == len(true_files) == 1, (generated_files, true_files)
        generated = read_rows(generated_files[0])
        hits = read_rows(true_files[0])
        assert len(generated) == len(hits) == config["n"], "Missing or duplicated throws/target hits"
        by_event = {row["evn"]: row for row in generated}
        assert len(by_event) == len(generated)
        assert {row["evn"] for row in hits} == set(by_event), "Hit event IDs disagree with generator"
        for hit in hits:
            assert hit["detector"] == "flux" and float(hit["tid"]) == 1 and float(hit["mtid"]) == 0
            check_momentum(hit, by_event[hit["evn"]])
            for axis in "xyz":
                key = "v" + axis
                assert math.isclose(float(hit[key]), float(by_event[hit["evn"]][key]), abs_tol=0.001), (
                    f"Analyzer {key} disagrees with thrown vertex in event {hit['evn']}")
        for plot in config["ganalysis"]:
            assert plot["plugin"] == "flux" and plot["source"] == "true"
            assert plot["x"] in hits[0], f"Missing Analyzer variable: {plot['x']}"
            if plot.get("dimension") == "2d":
                assert plot["y"] in hits[0], f"Missing Analyzer variable: {plot['y']}"
        check_vertices(generated, config["gparticle"][0])
        check_angles(generated, config["gparticle"][0])
    print(f"{card.stem}: {config['n']} throws passed; Analyzer vertices and momenta match generated records.")


if __name__ == "__main__":
    main()
