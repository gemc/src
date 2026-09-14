# Legacy JLAB SRO reference and CLAS12 migration

This directory is retained as reference source for the future `clas12-systems` SRO implementation.
Its six C++ files are preserved unchanged from GEMC revision
`47f47c68a7d43157aa5345be117519fce0d93090`. They are excluded from the build and use the retired streamer API.
The active GEMC factory is named `sro`.

The initial FT-Cal implementation now lives in `clas12-systems/geometry_src/ft/plugin/ft_cal`, with shared
JLAB payload/encoding definitions in `clas12-systems/geometry_src/sro`. It uses GEMC's generic worker dispatch
and crate threads, with local format fixtures and an offline CCDB-to-crate integration test.
Preserve this reference until output is validated with production TT assignments and a trusted JLAB reader.
The simple CSV example remains a separate introduction to the generic interfaces.

## Information preserved here

- [Factory header](gstreamerJLABSROFactory.h): packed `DataFrameHeader`, `.ev` extension, and the helper
  that exchanges the two 32-bit halves of a 64-bit value.
- [Frame assembly](stream/stream.cc): file preamble, header values, slot directory, and hit-word encoding.
- [Header writer](stream/frameHeader.cc) and [payload writer](stream/publishPayload.cc): serialized layout.
- [Connection](gstreamerJLABSROConnection.cc) and [factory](gstreamerJLABSROFactory.cc): old output lifecycle
  and loader entry point, for historical context.

The following describes what this code writes; it is not an independently verified format specification.

| Component | Legacy behavior |
|---|---|
| Preamble | Writes `0xC0DA2019`, `0xC0DA0001` when frame ID is 1. |
| Header layout | Seven packed `uint32_t` fields followed by three packed `uint64_t` fields: 52 bytes. |
| 32-bit fields, in order | `source_id`, `total_length`, `payload_length`, `compressed_length`, `magic`, |
| 32-bit fields, continued | `format_version`, `flags`. |
| 64-bit fields, in order | `record_counter`, `ts_sec`, `ts_nsec`. |
| Fixed header values | Source 0, magic `0xC0DA2019`, version 257, flags 0. |
| Counter and timestamps | Swaps 32-bit halves; timestamp is `frameID * 65536` ns, split into seconds/ns. |
| Payload prefix | `0x80000000`, then 16 slot-directory words for slots 0 through 15. |
| Slot directory entry | `(word_count << 16) \| offset`; offset counts words from the payload start. |
| Nonempty slot marker | `0x80008000 \| (crate << 8) \| slot`; included in the slot word count. |
| Input hit values | Integral vector `[crate, slot, channel, charge, time]`. |
| Hit word | `charge \| (channel << 13) \| ((time / 4) << 17)`. |
| Empty slot | Removes its marker; directory entry has zero count and retains its calculated offset. |
| Length fields | Payload bytes; compressed length equals payload bytes; total length is payload + 52 - 4. |

## Port plan for `clas12-systems`

1. Define an owned `GSROData` payload with crate, slot, channel, charge/ADC, and hit timing. Construct it in
   the detector worker's `stream_hit` hook, resolving `hit->getTTID()` through the `GTranslationTable`
   populated by `loadTTImpl`. Dispatch it through `GSROEmit`. Document units and ranges.
2. Supply a `GSROImplementation` and `GSROTiming` with a valid lower bound on all remaining payload times.
   Define frame duration, time origin, counter numbering, and boundary ownership explicitly.
3. Implement crate frame assembly in `GSROCratePlugin`, collecting all workers and channels for that crate.
   Keep overlap, electronics response, and unfinished-frame policy in the experiment implementation.
4. Implement the binary encoder in `GSROFrameSink`, with one output file per crate/run and checked binary
   writes. Load this implementation with `format: sro`; keep the GEMC factory name generic.
5. Validate bytes against a trusted JLAB reader or reference fixtures, then run the complete GEMC path with
   multiple crates, slots, and channels using one and several workers. Cover exact frame boundaries,
   empty slots, incomplete final frames, interruption, and file-open/write failures.

Use [the simple example](../../../../examples/basic/sro/README.md) for the current API and lifecycle.

## Decisions and legacy issues to resolve before compatibility is claimed

- The old slot marker uses `crate` before reading the current slot's hits. It starts at zero and can retain
  the last visited hit's crate. Use the owning crate's ID in the new encoder and verify expected source IDs.
- The writer serializes native memory and swaps 64-bit halves; this is not a complete byte-order conversion.
  Specify byte order and serialize fixed-width integers explicitly, without reinterpreting a word buffer as
  a packed header. Verify the unusual total-length convention and whether compression is required.
- Writing the preamble only for frame ID 1 can omit it when that frame has no output. Establish the actual
  file-start contract and frame numbering from trusted format evidence.
- The legacy shifts suggest field boundaries but apply no masks or range checks. Confirm channel/charge/time
  widths, the 4 ns time quantization, whether hit time is frame-relative, and overflow behavior.
- The 65536 ns timestamp increment is hard-coded here; confirm the required frame duration and timestamp
  semantics. Do not inherit the old per-worker output lifecycle or master-thread event gathering.

The initial FT-Cal port implements integral-hit frames; overlap/electronics extensions and production-reader
validation remain pending. These files preserve the original reference without enabling the retired backend.
