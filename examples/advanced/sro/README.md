# Simple streaming readout

**Upcoming in the next release.** This example sends a geantino through four scoring planes, representing
two channels in each of two crates. Each worker creates random ADC samples; a separate thread for each
crate collects all its channels into time frames and writes one CSV file.

Start with [sro_payload.h](sro_payload.h). The complete sample has just four fields:

| Field | Meaning | Example values |
|---|---|---|
| `crate` | Destination crate | 1, 2 |
| `slot` | Module slot inside that crate | 3 |
| `channel` | Input channel inside that slot | 0, 1 |
| `adc` | Random 12-bit ADC count | 0–4095 |

`AdcPayload` derives from `GSROData` and owns one sample. GEMC places it in a `GSROPayload` envelope that
also contains the routing crate, event ID, sequence number, and timestamp. Repeating the crate in the sample
makes the output record self-contained. `AdcFrame` owns the vector of samples collected for a time frame.

The [plugin](sro_plugin.cc) keeps the four stages together, with matching numbered comments:

1. `Digitizer::loadTTImpl` fills the inherited `GTranslationTable` before the run. `stream_hit` looks up
   `hit->getTTID()` with `translationTable->getElectronics()`, copies crate/slot/channel from `getHAddress()`,
   generates a random ADC, and calls `emit` immediately.
   The RNG is local and seeded from the event and channel address, so scheduling across workers does not change
   the output. It uses a fixed example seed, independently of GEMC's physics RNG and `-seed` option.
   Multiple hits at the same address in one event would receive the same ADC; this geometry produces one hit
   per channel/event.
2. `Timing` tells GEMC how early a sample that has not arrived yet could be. This helps the crate decide
   when it has everything needed to write a frame. In this example, all samples from event 0 have time 0 ns,
   event 1 has time 10 ns, event 2 has time 20 ns, and so on. These times come from
   `eventTimeWidth: 10*ns`; the example deliberately ignores the geantino's travel time.
3. `CsvSink` opens, writes, and closes one file on its owning crate thread.
4. `Crate` groups samples into 40 ns frames and writes only frames whose end is at or before safe time.
   `finish_run` discards any unfinished frames. No overlap, dead time, or ADC calibration is modeled.

GEMC supplies event completion and delivery acknowledgments. The plugin defines the 10 ns sampling period,
40 ns frame width, CSV encoding, and unfinished-frame policy. The same library exports both the digitizer
and `GSROImplementation` factories, making it a small starting point for an experiment implementation.

## Events, frames, and knowing when to write

An event and a frame are different things. An event is one simulated interaction. A frame collects samples
whose signal times fall in a chosen time interval, regardless of which event produced them.

Here, event starts are 10 ns apart and frames are 40 ns long. Frame 0 contains times from 0 ns up to, but
not including, 40 ns. Frame 1 contains times from 40 ns up to, but not including, 80 ns.

In a detector model that includes travel or electronics delays, **one event can contribute to several
frames**. For example, event 3 starts at 30 ns. A hit delayed by 5 ns has signal time 35 ns and goes into
frame 0; another hit delayed by 15 ns has signal time 45 ns and goes into frame 1. The random-ADC example
uses zero delay for every sample, so it does not demonstrate this split itself.

Receiving a sample at 45 ns does not mean frame 0 is ready: another worker could still send a sample at
35 ns. GEMC waits until events 0–3 have finished and all their samples have reached the crate buffers.
If no signal can occur before its event starts, the earliest possible sample still to come is then at
40 ns, from event 4. After processing all samples before 40 ns, the crate can write frame 0. Samples at
45 ns stay in frame 1, waiting for that frame to become complete.

This earliest possible time is called `safe_time`. It is a guarantee about samples still to come, not the
latest sample time seen so far. If a detector's timing corrections can move signals before their event
starts, its `Timing` implementation must allow for that too. See
[The timing contract](https://gemc.github.io/home/documentation/sro/plugins/#the-timing-contract).

## Sensitive IDs and electronics mapping

The geometry assigns each plane one sensitive identifier, `id`. The plugin's `loadTTImpl` defines its
electronics address:

| Sensitive ID (`getTTID()` key) | Crate | Slot | Channel |
|---|---|---|---|
| `{101}` | 1 | 3 | 0 |
| `{102}` | 1 | 3 | 1 |
| `{103}` | 2 | 3 | 0 |
| `{104}` | 2 | 3 | 1 |

GEMC invokes `loadTTImpl(runno, variation)` through `loadTT` before starting the run. This example uses a
fixed mapping for all runs and variations. The hook fills a local table and assigns it to the inherited
`translationTable` member, which workers read during the run. A missing identity is a translation-table error.
For identities with multiple components, the table key must match the values and order returned by `getTTID()`.

## Build and run from the source tree

With the Geant4 build module loaded, run these commands from the GEMC repository root:

```sh
meson setup --reconfigure build --wrap-mode=nodownload
meson compile -C build gemc example_sro
```

Generate the geometry and run in a scratch directory to keep generated files out of the source tree:

```sh
gemc_src="$PWD"
sro_run=$(mktemp -d /tmp/gemc-sro-example.XXXXXX)
cp "$gemc_src/examples/advanced/sro/sro.yaml" "$sro_run/"
cd "$sro_run"
"$gemc_src/build/subprojects/pygemc/python_env/bin/python" \
  "$gemc_src/examples/advanced/sro/sro.py" -f ascii
"$gemc_src/build/bin/gemc" sro.yaml \
  -plugin_path="$gemc_src/build/examples/advanced/sro"
```

The supplied YAML selects `format: sro` with `implementation: example_sro`, sets `eventTimeWidth: 10*ns`,
runs 10 events on four workers, and enables `recordZeroEdep` so the non-interacting geantino produces hits.
The plugin is built as
`example_sro.gplugin` and is also installed with GEMC. In an installed tree, use the installed Python
environment, `bin/gemc`, and plugin search path instead of the build paths above.

## Read the output

The default run creates `simple_sro_r1_crate1.csv` and `simple_sro_r1_crate2.csv`. GEMC sets Geant4's run ID
from `runno: 1`; the plugin includes that ID in the filename. There are no per-worker SRO files.

Each CSV row contains:

```text
frame_id,begin_ns,end_ns,crate,slot,channel,adc
```

For the default 10-event run, each crate file has 16 sample rows:

| Frame | Time interval | Events | Samples per crate | Written? |
|---|---|---|---|---|
| 0 | [0, 40) ns | 0–3 | 8 | Yes |
| 1 | [40, 80) ns | 4–7 | 8 | Yes |
| 2 | [80, 120) ns | 8–9 | 4 | No: safe time only reaches 100 ns |

A sample exactly at 40 ns belongs to frame 1. CSV row order follows GEMC's `(time, event, sequence)` order.
ADC values vary; the same events and addresses reproduce the same values on the same C++ toolchain.

Try `-n=8` for two complete frames, or `-n=3` for files containing only the CSV header. Changing `-nthreads`
changes the worker count without changing the completed output frames. Re-running the same invocation with
the same output basename overwrites its files; use a different `gstreamer.filename` to keep multiple runs.

## Check the example

From the repository root:

```sh
meson test -C build examples_sro_adc --print-errorlogs
```

The check generates geometry with sensitive IDs and runs GEMC in a temporary directory. It checks the
translation-table mapping to both crates and channels, ADC bounds and variation, frame boundaries,
identical output with one and four workers, and discard of partial frames.
