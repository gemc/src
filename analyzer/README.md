# GEMC Analyzer

`analyzer` is a small Python package for reading GEMC output files and plotting
variables by name. It currently focuses on CSV and ROOT output from `gstreamer`,
with a reader structure that can be extended to other formats later.

## Dependencies

The main analyzer API uses the standard scientific Python stack:

```sh
python3 -m pip install pandas numpy matplotlib
```

ROOT output support also requires `uproot`:

```sh
python3 -m pip install uproot
```

ROOT prerequisites:

- GEMC must be built with ROOT support and the ROOT streamer plugin available.
- The run must use `gstreamer` format `root`.
- Reading ROOT files from Python does not require importing C++ ROOT; the analyzer
  uses `uproot`.

The dependency-free SVG helper in `analyzer/svg_plot.py` only uses the Python
standard library. It is useful on minimal systems where `pandas`, `numpy`, and
`matplotlib` are not installed.

## GEMC Output Model

The CSV streamer writes two flattened files per worker thread:

```text
<rootname>_t<thread>_true_info.csv
<rootname>_t<thread>_digitized.csv
```

For one thread and `filename: b2`, the files are typically:

```text
b2_t0_true_info.csv
b2_t0_digitized.csv
```

CSV files are read with:

```python
pd.read_csv(path, sep=",", skipinitialspace=True)
```

The CSV rows include event context columns such as:

```text
evn, timestamp, thread_id, detector
```

The digitized B2 output includes columns like:

```text
hitn, pid, tid, E, time, totEdep
```

The true-info output includes tracking columns like:

```text
processName, avgTime, avgx, avgy, avgz, hitn, pid, tid, mtid, vx, vy, vz, mvx, mvy, mvz, totalEDeposited
```

When the matching digitized CSV is available, the analyzer also adds `E` to
true-info tables by matching rows on event, detector, hit, PID, and track ID.
In that case `E` is the track total energy, while `totalEDeposited` remains
the deposited energy.

The `vx`, `vy`, and `vz` columns are the current track vertex coordinates.
The `mvx`, `mvy`, and `mvz` columns are the mother-track vertex coordinates
when the mother track was available to GEMC hit processing; otherwise they use
the GEMC uninitialized numeric sentinel. The `mtid` column stores the mother
track id.

The ROOT streamer writes one ROOT file per worker thread. For one thread and
`filename: b2`, the file is typically:

```text
b2_t0.root
```

The file contains trees named:

```text
event_header
run_header
true_info_<detector>
digitized_<detector>
```

ROOT detector trees store vector branches. The analyzer flattens each vector
element into one `DataFrame` row.

## Python API

Read one digitized CSV file:

```python
from analyzer import read_output

output = read_output("b2_t0_digitized.csv")
df = output.get_frame("digitized")
print(df.columns)
```

Read a CSV root name when both files exist:

```python
from analyzer import read_output

output = read_output("b2_t0", kind="csv")
print(output.summary())
```

Plot `totEdep` grouped by `pid`:

```python
from analyzer import plot_variable, read_output

output = read_output("b2_t0_digitized.csv")
plot_variable(
    output,
    "totEdep",
    data="digitized",
    bins=30,
    xlim=(0.0, 0.1),
    show=True,
)
```

Read ROOT output:

```python
from analyzer import read_output

output = read_output("b2_t0.root", kind="root")
df = output.get_frame("digitized", detector="flux")
```

## Command-Line Usage

Run `python3 -m analyzer` from the GEMC source directory, where the `analyzer`
package directory is visible to Python.

The `-m` flag takes a module name, not a filesystem path. Do not run
`python3 -m ../analyzer`. If your shell is in another directory, move back to
the source directory or set `PYTHONPATH`.

Print a summary:

```sh
python3 -m analyzer digitized.csv
```

Plot a digitized variable with matplotlib:

```sh
python3 -m analyzer digitized.csv totEdep --kind csv --xlim 0.0 0.1
```

Save a plot instead of showing it:

```sh
python3 -m analyzer digitized.csv totEdep --kind csv --save b2_totEdep.png
```

Plot ROOT output with matplotlib:

```sh
python3 -m analyzer b2_t0.root totEdep --kind root --detector flux --save b2_totEdep.png
```

Plot a true-info track vertex coordinate:

```sh
python3 -m analyzer true_info.csv vx --kind csv --data true_info --save b2_vertex_x.png
```

## Dependency-Free SVG Plot

If `pandas`, `numpy`, or `matplotlib` are unavailable, create an SVG histogram
directly from the CSV file:

```sh
python3 -B analyzer/svg_plot.py b2_t0_digitized.csv totEdep --out b2_totEdep.svg --bins 30
```

Add an x-axis range with:

```sh
python3 -B analyzer/svg_plot.py b2_t0_digitized.csv totEdep --out b2_totEdep.svg --bins 30 --xlim 0.0 0.1
```

## Run the B2 Example

Run these commands from the GEMC source directory.

Build the B2 geometry into a local SQLite database:

```sh
PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=/opt/projects/gemc/src/api \
python3 examples/basic/b2/b2.py -f sqlite -sql gemc.db
```

Run GEMC with CSV output rooted at `b2`:

```sh
build/gemc examples/basic/b2/b2.yaml \
  '-gsystem=[{name: b2, factory: sqlite}]' \
  '-gstreamer=[{format: csv, filename: b2}]' \
  -sql=gemc.db \
  -n=20
```

With one worker thread, this produces:

```text
b2_t0_digitized.csv
b2_t0_true_info.csv
```

Inspect the digitized CSV header:

```sh
head -1 b2_t0_digitized.csv
```

Expected columns include:

```text
evn, timestamp, thread_id, detector, hitn, pid, tid, E, time, totEdep
```

Create the `totEdep` plot with the main analyzer API:

```sh
python3 -m analyzer digitized.csv totEdep --kind csv --save b2_totEdep.png
```

Or create the same style of histogram without third-party Python packages:

```sh
python3 -B analyzer/svg_plot.py b2_t0_digitized.csv totEdep --out b2_totEdep.svg --bins 30
```

### Run B2 With ROOT Output

To produce ROOT output instead of CSV, keep the same `gemc.db` and run:

```sh
build/gemc examples/basic/b2/b2.yaml \
  '-gsystem=[{name: b2, factory: sqlite}]' \
  '-gstreamer=[{format: root, filename: b2}]' \
  -sql=gemc.db \
  -n=20
```

With one worker thread, this produces:

```text
b2_t0.root
```

Read the ROOT file from Python if you want to inspect or manipulate the data
before plotting:

```python
from analyzer import plot_variable, read_output

output = read_output("b2_t0.root", kind="root")
print(output.summary())

df = output.get_frame("digitized", detector="flux")
print(df[["pid", "totEdep"]].head())

plot_variable(
    output,
    "totEdep",
    data="digitized",
    detector="flux",
    bins=30,
    show=True,
)
```

The Python inspection step is not required for plotting. To plot directly from
the command line, use:

```sh
python3 -m analyzer b2_t0.root totEdep --kind root --detector flux --save b2_root_totEdep.png
```

If matplotlib reports that its default cache directory is not writable, set a
writable `MPLCONFIGDIR`:

```sh
MPLCONFIGDIR=. python3 -m analyzer b2_t0.root totEdep --kind root --detector flux --save b2_root_totEdep.png
```

## Extending Readers

New formats should return a `GemcOutput` object from `analyzer.dataset`.
Populate one or more of these maps:

```python
GemcOutput(
    true_info={"name": true_info_dataframe},
    digitized={"name": digitized_dataframe},
    headers={"event_header": event_header_dataframe},
)
```

Then add the format selection to `read_output()` in `analyzer/readers.py`.
