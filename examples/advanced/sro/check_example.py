#!/usr/bin/env python3
"""Exercise sensitive-ID translation, worker payloads, crate grouping, and incomplete-frame discard."""

import csv
from collections import Counter
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def run(command, directory):
    result = subprocess.run(command, cwd=directory, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, timeout=30)
    if result.returncode:
        raise RuntimeError(f"Command failed ({result.returncode}): {command}\n{result.stdout[-6000:]}")


def main():
    gemc, plugin = (Path(arg).resolve() for arg in sys.argv[1:])
    example = Path(__file__).resolve().parent
    with tempfile.TemporaryDirectory(prefix="gemc-sro-adc-") as temporary:
        directory = Path(temporary)
        shutil.copyfile(example / "sro.yaml", directory / "sro.yaml")
        run([sys.executable, str(example / "sro.py"), "-f", "ascii"], directory)
        reference = None
        for workers, events in ((1, 10), (4, 10), (4, 8), (4, 3)):
            base = directory / f"adc_w{workers}_n{events}"
            output = f"[{{format: sro, filename: '{base}', implementation: example_sro}}]"
            run([str(gemc), "sro.yaml", f"-plugin_path={plugin.parent}",
                 f"-nthreads={workers}", f"-n={events}", f"-gstreamer={output}"], directory)
            files = sorted(directory.glob(base.name + "*.csv"))
            expected_files = [Path(f"{base}_r1_crate{crate}.csv") for crate in (1, 2)]
            assert files == expected_files, files  # One file per crate; no worker suffixes.
            results = []
            for crate, filename in enumerate(files, 1):
                with filename.open(newline="") as stream:
                    reader = csv.DictReader(stream)
                    assert reader.fieldnames == ["frame_id", "begin_ns", "end_ns", "crate",
                                                "slot", "channel", "adc"]
                    rows = [{key: int(value) for key, value in row.items()} for row in reader]
                complete_frames = events // 4
                assert len(rows) == complete_frames * 8, rows
                # Geometry supplies only sensitive IDs; these addresses must come from loadTTImpl.
                counts = Counter((row["frame_id"], row["channel"]) for row in rows)
                assert counts == Counter({(frame, channel): 4 for frame in range(complete_frames)
                                          for channel in (0, 1)}), counts
                assert [row["frame_id"] for row in rows] == sorted(row["frame_id"] for row in rows)
                for row in rows:
                    assert row["crate"] == crate and row["slot"] == 3, row
                    assert row["begin_ns"] == row["frame_id"] * 40, row
                    assert row["end_ns"] == row["begin_ns"] + 40, row
                    assert 0 <= row["adc"] <= 4095, row
                if rows:
                    assert len({row["adc"] for row in rows}) > 1
                results.append(rows)
            if reference is None:
                reference = results
            elif events >= 8:
                assert results == reference, "Worker scheduling or discarded tail changed complete frames"
    print("Simple SRO ADC example passed: crate/channel grouping, random ADCs, and incomplete-frame discard.")


if __name__ == "__main__":
    main()
