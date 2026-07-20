#!/usr/bin/env python3

"""Generate consistent SVG workflow diagrams for every documented GEMC module."""

from __future__ import annotations

import argparse
import textwrap
import xml.etree.ElementTree as ET
from pathlib import Path


Step = tuple[str, str, str]
Diagram = tuple[str, str, tuple[Step, ...]]

DIAGRAMS: dict[str, Diagram] = {
    "actions": (
        "Geant4 action lifecycle",
        "Callbacks connect Geant4 execution to GEMC event data and output.",
        (
            ("Geant4 callbacks", "Run, event, tracking, stepping, and generator hooks", "input"),
            ("Action setup", "GActionInitialization installs worker-local actions", "core"),
            ("Runtime actions", "Primary, run, event, stepping, and tracking logic", "action"),
            ("Data collection", "Generated particles, hits, event and run records", "core"),
            ("Publication", "Thread-local streamers write analysis products", "output"),
        ),
    ),
    "dbselect": (
        "Detector setup selection",
        "The setup model turns database choices into a geometry reload request.",
        (
            ("Experiment catalog", "Available systems, variations, and run ranges", "input"),
            ("Setup model", "Rows group experiment and detector choices", "core"),
            ("User selection", "Include systems and choose variation and run", "action"),
            ("Validation", "Status indicators identify complete selections", "core"),
            ("Geometry reload", "Selected configuration rebuilds the detector", "output"),
        ),
    ),
    "eventDispenser": (
        "Run-weighted event dispatch",
        "Requested events are allocated across runs before Geant4 processing.",
        (
            ("Event request", "Total count, run number, or run-weight file", "input"),
            ("Weight parsing", "Read and normalize relative run weights", "core"),
            ("Run allocation", "Sample the cumulative distribution", "action"),
            ("Run setup", "Reload constants and translation tables", "plugin"),
            ("Beam dispatch", "Issue one beamOn command for each run", "output"),
        ),
    ),
    "g4dialog": (
        "Geant4 command workflow",
        "The command browser connects discovery, help, history, and execution.",
        (
            ("Command tree", "Geant4 UI directories and commands", "input"),
            ("Search", "Filter paths and select a command", "action"),
            ("Context help", "Show guidance and parameter descriptions", "core"),
            ("Prompt and history", "Edit commands and recall prior entries", "action"),
            ("UI manager", "Apply the selected Geant4 command", "output"),
        ),
    ),
    "g4display": (
        "Interactive visualization control",
        "Display options and Qt controls are translated into Geant4 viewer commands.",
        (
            ("Display options", "Driver, camera, lighting, cuts, and decorations", "input"),
            ("Scene setup", "Create the viewer and initialize scene properties", "core"),
            ("Qt controls", "View and Utilities tabs capture user changes", "action"),
            ("Command mapping", "Build Geant4 visualization commands", "core"),
            ("Live viewer", "Refresh camera, geometry, labels, and styles", "output"),
        ),
    ),
    "g4system": (
        "Geant4 geometry construction",
        "GEMC system records become a complete Geant4 placement hierarchy.",
        (
            ("GWorld records", "Systems, volumes, materials, and hierarchy", "input"),
            ("Materials", "Resolve elements, mixtures, and optical properties", "core"),
            ("Solids", "Build primitives, CAD meshes, and Boolean solids", "core"),
            ("Logical volumes", "Attach materials, visualization, fields, and SDs", "action"),
            ("Placements", "Create the world and physical-volume tree", "output"),
        ),
    ),
    "gbase": (
        "Shared module foundation",
        "GBase gives derived components consistent configuration and logging behavior.",
        (
            ("Shared GOptions", "Resolved configuration and logger controls", "input"),
            ("GBase<T>", "Store module context and common facilities", "core"),
            ("Module logger", "Select category, verbosity, and debug level", "core"),
            ("Typed access", "Derived code reads validated option values", "action"),
            ("Component behavior", "Consistent diagnostics and configuration", "output"),
        ),
    ),
    "gboard": (
        "GUI log routing",
        "Geant4 console output becomes searchable, styled text inside the GUI.",
        (
            ("G4cout / G4cerr", "Geant4 emits console and diagnostic text", "input"),
            ("GUI session", "Capture, split, and route complete lines", "core"),
            ("ANSI conversion", "Translate terminal styles into safe HTML", "core"),
            ("GBoard view", "Append, filter, and search the log", "action"),
            ("User actions", "Clear the view or save the visible log", "output"),
        ),
    ),
    "gdata": (
        "Simulation data hierarchy",
        "Hit-level records are organized into detector, event, run, and frame collections.",
        (
            ("GHit", "Accumulated Geant4 step information", "input"),
            ("Hit records", "GTrueInfoData and GDigitizedData", "core"),
            ("Detector data", "GDataCollection groups records by detector", "core"),
            ("Aggregation", "Event, run, and frame collection layers", "action"),
            ("Streamer view", "Stable read-only data for output plugins", "output"),
        ),
    ),
    "gdetector": (
        "Detector construction orchestration",
        "Detector records, plugins, fields, and sensitive volumes converge in Geant4.",
        (
            ("Aggregated options", "Systems, geometry, digitization, SD, and fields", "input"),
            ("World loading", "GWorld and G4World build detector geometry", "core"),
            ("Sensitive detectors", "Create and map detector readout instances", "action"),
            ("Plugins and fields", "Bind digitizers and field managers to volumes", "plugin"),
            ("Geant4 detector", "Ready-to-run geometry and detector services", "output"),
        ),
    ),
    "gdynamicDigitization": (
        "Digitization plugin lifecycle",
        "A detector-specific routine transforms sensitive-volume hits into output banks.",
        (
            ("Sensitive volume", "Geometry selects a named digitization routine", "input"),
            ("Plugin creation", "Load built-in code or a dynamic plugin", "plugin"),
            ("Run preparation", "Define readout, load constants, and translation", "core"),
            ("Hit processing", "Filter, reshape, digitize, and apply response", "action"),
            ("Data products", "Digitized response and Monte-Carlo truth", "output"),
        ),
    ),
    "gfactory": (
        "Factory and plugin resolution",
        "Static registrations and dynamic libraries share one object-creation interface.",
        (
            ("Factory request", "Type, plugin name, and constructor context", "input"),
            ("Static registry", "Look for an in-process registered creator", "core"),
            ("Plugin loader", "Open a shared library and resolve its factory", "plugin"),
            ("Object creation", "Invoke the selected typed constructor", "action"),
            ("Managed instance", "Keep plugin lifetime tied to the object", "output"),
        ),
    ),
    "gfields": (
        "Magnetic-field installation",
        "Field definitions become plugin instances and Geant4 field managers.",
        (
            ("Field options", "Named parameters, units, axes, and integration", "input"),
            ("Field definition", "Normalize configuration for a plugin", "core"),
            ("Field factory", "Load and initialize the concrete field", "plugin"),
            ("Field manager", "Create equation, stepper, and chord finder", "action"),
            ("Volume attachment", "Evaluate the field during particle transport", "output"),
        ),
    ),
    "ghit": (
        "Hit accumulation",
        "Multiple Geant4 steps are accumulated into one detector-cell hit.",
        (
            ("G4Step", "Position, time, energy, track, and process", "input"),
            ("Touchable key", "Identify the detector cell and discriminator", "core"),
            ("Step vectors", "Append local and global per-step quantities", "action"),
            ("Derived values", "Calculate totals, averages, and representative data", "core"),
            ("Final GHit", "Input for truth collection and digitization", "output"),
        ),
    ),
    "glogging": (
        "Structured log emission",
        "Logger configuration gates and formats messages before Geant4 emits them.",
        (
            ("Log call", "Category, level, code, and message arguments", "input"),
            ("Policy lookup", "Read verbosity and debug for the logger name", "core"),
            ("Level gate", "Suppress messages above the configured level", "action"),
            ("Header format", "Add category, severity, and message counter", "core"),
            ("G4 output", "Write consistent lines to G4cout or G4cerr", "output"),
        ),
    ),
    "goptions": (
        "Configuration resolution",
        "Schemas, YAML, and command-line values become one validated configuration.",
        (
            ("Option schemas", "Switches, scalar values, structured nodes, defaults", "input"),
            ("Configuration input", "Read YAML files and command-line arguments", "input"),
            ("Merge", "Apply precedence and dot-notation updates", "action"),
            ("Validation", "Check types, mandatory keys, and value constraints", "core"),
            ("Resolved options", "Typed access plus a reproducible YAML snapshot", "output"),
        ),
    ),
    "gparticle": (
        "Primary-particle production",
        "Inline definitions or event files become Geant4 primaries and metadata banks.",
        (
            ("Particle source", "Inline options or file-backed event records", "input"),
            ("Source parsing", "Resolve species, multiplicity, and source type", "core"),
            ("Phase-space sample", "Momentum, angles, and vertex distributions", "action"),
            ("Geant4 primary", "Create vertices and particles for transport", "core"),
            ("Generated banks", "Tracked and source-complete particle metadata", "output"),
        ),
    ),
    "gphysics": (
        "Physics-list construction",
        "Runtime selections customize a Geant4 reference physics list.",
        (
            ("Physics option", "Reference list, EM choice, and extra constructors", "input"),
            ("Reference factory", "Create the requested Geant4 physics list", "core"),
            ("EM replacement", "Optionally replace electromagnetic physics", "action"),
            ("Extensions", "Register optical or other physics constructors", "plugin"),
            ("Physics list", "Configured G4VModularPhysicsList for the run", "output"),
        ),
    ),
    "gqtbuttonswidget": (
        "Reusable Qt button controls",
        "Icon resources and labels become palette-aware button signals.",
        (
            ("Button definition", "SVG resource paths, labels, size, and layout", "input"),
            ("Palette render", "Resolve normal, selected, and highlighted colors", "core"),
            ("Qt widgets", "Create icon-list or checkable push buttons", "core"),
            ("Interaction", "Press, toggle, reset, or select from code", "action"),
            ("Index signal", "Notify the owning page of the new state", "output"),
        ),
    ),
    "gsd": (
        "Sensitive-detector step processing",
        "Each Geant4 step is filtered, keyed, and accumulated into an event hit collection.",
        (
            ("G4Step", "A step occurs inside a sensitive logical volume", "input"),
            ("Early decision", "The digitizer may skip the energy deposit", "plugin"),
            ("Touchable processing", "Map the step to one or more detector cells", "action"),
            ("Create or merge", "Append data to the matching event GHit", "core"),
            ("Hits collection", "Geant4 owns the completed event collection", "output"),
        ),
    ),
    "gsplash": (
        "Splash-screen lifecycle",
        "GUI policy and image resolution produce a timed startup display.",
        (
            ("GUI policy", "Check the gui switch and splash options", "input"),
            ("Image lookup", "Try themed resource, file path, or Qt resource", "core"),
            ("Pixmap setup", "Load, scale, and create QSplashScreen", "core"),
            ("Status messages", "Show immediate or delayed initialization text", "action"),
            ("Timed close", "Respect minimum duration and release the widget", "output"),
        ),
    ),
    "gstreamer": (
        "Plugin-based data publication",
        "Common publish hooks serialize event, run, and frame data through selected backends.",
        (
            ("Data collections", "Event, run, frame, generated, and ancestor data", "input"),
            ("Event buffer", "Hold shared event data until the flush limit", "core"),
            ("Publish sequence", "Drive common headers, detector banks, and trailers", "action"),
            ("Format plugin", "ASCII, CSV, JSON, ROOT, or JLAB SRO", "plugin"),
            ("External output", "Files or binary records per worker and stream", "output"),
        ),
    ),
    "gsystem": (
        "Detector-system loading",
        "Geometry sources become modified, named systems assembled in the GEMC world.",
        (
            ("System options", "Name, factory, variation, run, and source", "input"),
            ("Source factory", "Load SQLite, ASCII, CAD, or GDML records", "plugin"),
            ("System model", "Create materials and placed-volume records", "core"),
            ("World modifiers", "Apply shifts, tilts, and existence rules", "action"),
            ("GWorld", "Final system map and globally unique volume names", "output"),
        ),
    ),
    "gtouchable": (
        "Detector-cell identity",
        "Identity fields and detector-specific discriminators form a stable hit key.",
        (
            ("G4 touchable", "Volume history and detector identifiers", "input"),
            ("Identity vector", "Ordered name and integer identifier pairs", "core"),
            ("Discriminator", "Time bin, track, particle, or identity only", "plugin"),
            ("Equality test", "Compare identity plus detector-type context", "action"),
            ("Cell key", "Create or find the hit to accumulate", "output"),
        ),
    ),
    "gtranslationTable": (
        "Electronics-channel lookup",
        "Detector identities are normalized into stable electronics mappings.",
        (
            ("Identity vector", "Detector-specific integer coordinates", "input"),
            ("Stable key", "Serialize the ordered values with separators", "core"),
            ("Registration", "Associate the key with GElectronic data", "action"),
            ("Lookup", "Resolve the same key during digitization", "core"),
            ("Electronics data", "Crate, slot, channel, and detector metadata", "output"),
        ),
    ),
    "gtree": (
        "Interactive geometry inspection",
        "Volume maps become a controllable hierarchy linked to the Geant4 viewer.",
        (
            ("Volume maps", "G4Volume wrappers and source GVolume metadata", "input"),
            ("Tree model", "Group systems, parents, children, and visibility", "core"),
            ("Qt hierarchy", "Display names, colors, and checkboxes", "core"),
            ("Style controls", "Change representation, opacity, and focus", "action"),
            ("Inspection", "Show parameters, isolate volumes, and find overlaps", "output"),
        ),
    ),
    "gui": (
        "Top-level GUI coordination",
        "Navigation pages and run controls coordinate the interactive simulation.",
        (
            ("Shared services", "Options, detector construction, and event dispenser", "input"),
            ("Page navigation", "Display, setup, volumes, dialog, and generator", "core"),
            ("Content stack", "Create and switch the active Qt tool page", "core"),
            ("Run controls", "Run, cycle, stop, exit, and event counters", "action"),
            ("Simulation state", "Keep geometry, viewer, logs, and events synchronized", "output"),
        ),
    ),
    "guts": (
        "Dependency-light utility flow",
        "Common text, numeric, path, and filesystem operations share predictable helpers.",
        (
            ("Raw values", "Strings, paths, tokens, and unit expressions", "input"),
            ("Normalization", "Trim whitespace and replace delimiters", "core"),
            ("Parsing", "Split tokens and convert numbers and units", "action"),
            ("Filesystem helpers", "Resolve paths and enumerate matching files", "core"),
            ("Stable result", "Reusable values with consistent error behavior", "output"),
        ),
    ),
    "textProgressBar": (
        "Terminal progress reporting",
        "Loop counters and timing data become a stable single-line progress display.",
        (
            ("Loop update", "Current item and total work count", "input"),
            ("Progress state", "Compute fraction, elapsed time, and rate", "core"),
            ("Bar layout", "Fit percentage and fill glyphs to fixed width", "core"),
            ("Terminal render", "Rewrite one line without growing the log", "action"),
            ("Completion", "Render 100 percent and terminate the line", "output"),
        ),
    ),
    "utilities": (
        "Runtime utility services",
        "Small process-level helpers prepare and supervise a GEMC run.",
        (
            ("Application startup", "Options, environment, and process state", "input"),
            ("Runtime setup", "Random seed, signals, and Geant4 output", "core"),
            ("Execution helpers", "Issue commands and coordinate shared services", "action"),
            ("Failure handling", "Translate signals and errors into clean shutdown", "core"),
            ("Run outcome", "Reproducible execution and diagnostic status", "output"),
        ),
    ),
}

OUTPUT_DIRECTORY = Path(__file__).with_name("doxygen-images")
NODE_WIDTH = 156
NODE_HEIGHT = 124
NODE_GAP = 40
NODE_Y = 104
KINDS = (("input", "Input"), ("core", "Core"), ("plugin", "Extension"), ("action", "Action"),
         ("output", "Output"))

STYLE = """
.background { fill: #ffffff; }
.title { fill: #1f2328; font: 700 23px Avenir, "Segoe UI", Arial, sans-serif; }
.description { fill: #59636e; font: 13px Avenir, "Segoe UI", Arial, sans-serif; }
.node { stroke-width: 1.5; }
.node.input { fill: #e8f1ff; stroke: #1d6ff2; }
.node.core { fill: #f0f2f4; stroke: #66717d; }
.node.plugin { fill: #fff3d6; stroke: #b77900; }
.node.action { fill: #ffe9e5; stroke: #c2412d; }
.node.output { fill: #e3f7ea; stroke: #238636; }
.step-number { fill: #ffffff; stroke-width: 1.5; }
.step-number.input { stroke: #1d6ff2; }
.step-number.core { stroke: #66717d; }
.step-number.plugin { stroke: #b77900; }
.step-number.action { stroke: #c2412d; }
.step-number.output { stroke: #238636; }
.number { fill: #1f2328; font: 700 12px Avenir, "Segoe UI", Arial, sans-serif; text-anchor: middle; }
.node-title { fill: #1f2328; font: 700 14px Avenir, "Segoe UI", Arial, sans-serif; text-anchor: middle; }
.node-detail { fill: #59636e; font: 11.5px Avenir, "Segoe UI", Arial, sans-serif; text-anchor: middle; }
.connector { fill: none; stroke: #7d8590; stroke-width: 2; marker-end: url(#arrow); }
.legend-text { fill: #59636e; font: 11px Avenir, "Segoe UI", Arial, sans-serif; }
.legend.input { fill: #e8f1ff; stroke: #1d6ff2; }
.legend.core { fill: #f0f2f4; stroke: #66717d; }
.legend.plugin { fill: #fff3d6; stroke: #b77900; }
.legend.action { fill: #ffe9e5; stroke: #c2412d; }
.legend.output { fill: #e3f7ea; stroke: #238636; }
@media (prefers-color-scheme: dark) {
  .background { fill: #0d1117; }
  .title, .number, .node-title { fill: #e6edf3; }
  .description, .node-detail, .legend-text { fill: #9da7b3; }
  .node.input { fill: #15243a; stroke: #58a6ff; }
  .node.core { fill: #21262d; stroke: #8b949e; }
  .node.plugin { fill: #352b18; stroke: #d29922; }
  .node.action { fill: #3a211d; stroke: #f47067; }
  .node.output { fill: #173326; stroke: #3fb950; }
  .step-number { fill: #0d1117; }
  .connector { stroke: #8b949e; }
  .legend.input { fill: #15243a; stroke: #58a6ff; }
  .legend.core { fill: #21262d; stroke: #8b949e; }
  .legend.plugin { fill: #352b18; stroke: #d29922; }
  .legend.action { fill: #3a211d; stroke: #f47067; }
  .legend.output { fill: #173326; stroke: #3fb950; }
}
""".strip()


def add_text(parent: ET.Element, css_class: str, x: int, y: int, lines: list[str], gap: int) -> None:
    """Add centered SVG text with one tspan per line."""

    text = ET.SubElement(parent, "text", {"class": css_class, "x": str(x), "y": str(y)})
    for index, line in enumerate(lines):
        attributes = {"x": str(x)}
        if index:
            attributes["dy"] = str(gap)
        ET.SubElement(text, "tspan", attributes).text = line


def render_diagram(module: str, diagram: Diagram) -> str:
    """Render one five-stage module workflow as standalone SVG."""

    title, description, steps = diagram
    if len(steps) != 5:
        raise ValueError(f"{module} must define exactly five workflow steps")

    ET.register_namespace("", "http://www.w3.org/2000/svg")
    root = ET.Element(
        "svg",
        {
            "xmlns": "http://www.w3.org/2000/svg",
            "viewBox": "0 0 1000 320",
            "role": "img",
            "aria-labelledby": "title desc",
        },
    )
    ET.SubElement(root, "title", {"id": "title"}).text = title
    ET.SubElement(root, "desc", {"id": "desc"}).text = f"A five-stage workflow for the {module} module."

    defs = ET.SubElement(root, "defs")
    marker = ET.SubElement(
        defs,
        "marker",
        {"id": "arrow", "markerWidth": "9", "markerHeight": "7", "refX": "8", "refY": "3.5",
         "orient": "auto"},
    )
    ET.SubElement(marker, "polygon", {"points": "0 0, 9 3.5, 0 7", "fill": "#7d8590"})
    ET.SubElement(defs, "style").text = STYLE
    ET.SubElement(root, "rect", {"class": "background", "width": "1000", "height": "320"})
    ET.SubElement(root, "text", {"class": "title", "x": "30", "y": "38"}).text = title
    add_text(root, "description", 30, 64, [description], 0)

    for index in range(4):
        start_x = 30 + index * (NODE_WIDTH + NODE_GAP) + NODE_WIDTH
        end_x = start_x + NODE_GAP - 8
        ET.SubElement(
            root,
            "path",
            {"class": "connector", "d": f"M {start_x + 4} 166 L {end_x} 166"},
        )

    for index, (step_title, detail, kind) in enumerate(steps):
        x = 30 + index * (NODE_WIDTH + NODE_GAP)
        center = x + NODE_WIDTH // 2
        group = ET.SubElement(root, "g")
        ET.SubElement(
            group,
            "rect",
            {"class": f"node {kind}", "x": str(x), "y": str(NODE_Y), "width": str(NODE_WIDTH),
             "height": str(NODE_HEIGHT), "rx": "6"},
        )
        ET.SubElement(
            group,
            "circle",
            {"class": f"step-number {kind}", "cx": str(x + 20), "cy": str(NODE_Y + 20), "r": "11"},
        )
        ET.SubElement(
            group,
            "text",
            {"class": "number", "x": str(x + 20), "y": str(NODE_Y + 24)},
        ).text = str(index + 1)
        add_text(group, "node-title", center, 148, textwrap.wrap(step_title, width=19), 17)
        add_text(group, "node-detail", center, 188, textwrap.wrap(detail, width=23), 15)

    legend_x = 166
    for kind, label in KINDS:
        ET.SubElement(
            root,
            "rect",
            {"class": f"legend {kind}", "x": str(legend_x), "y": "274", "width": "13", "height": "13",
             "rx": "2"},
        )
        ET.SubElement(root, "text", {"class": "legend-text", "x": str(legend_x + 20), "y": "285"}).text = label
        legend_x += 135

    ET.indent(root, space="  ")
    return '<?xml version="1.0" encoding="UTF-8"?>\n' + ET.tostring(root, encoding="unicode") + "\n"


def output_path(module: str) -> Path:
    return OUTPUT_DIRECTORY / f"{module.lower()}-flow.svg"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail when checked-in diagrams are stale")
    args = parser.parse_args()

    stale = []
    OUTPUT_DIRECTORY.mkdir(parents=True, exist_ok=True)
    for module, diagram in sorted(DIAGRAMS.items()):
        content = render_diagram(module, diagram)
        destination = output_path(module)
        if args.check:
            if not destination.exists() or destination.read_text(encoding="utf-8") != content:
                stale.append(destination)
        else:
            destination.write_text(content, encoding="utf-8")

    if stale:
        print("Doxygen diagrams are missing or stale:")
        for path in stale:
            print(f"  {path}")
        print("Run ci/generate_doxygen_diagrams.py to regenerate them.")
        return 1

    action = "checked" if args.check else "generated"
    print(f"Doxygen diagrams {action}: {len(DIAGRAMS)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
