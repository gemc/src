#!/usr/bin/env python3

"""Generate detailed SVG concept diagrams for selected Doxygen modules."""

from __future__ import annotations

import argparse
import math
from pathlib import Path
import textwrap
import xml.etree.ElementTree as ET


OUTPUT_DIRECTORY = Path(__file__).with_name("doxygen-images")

STYLE = """
.background { fill: #ffffff; }
.panel { fill: #f6f8fa; stroke: #d0d7de; stroke-width: 1.5; }
.panel-dashed { fill: #f6f8fa; stroke: #8c959f; stroke-dasharray: 7 5; stroke-width: 1.5; }
.title { fill: #1f2328; font: 700 23px Avenir, "Segoe UI", Arial, sans-serif; }
.subtitle { fill: #59636e; font: 13px Avenir, "Segoe UI", Arial, sans-serif; }
.heading { fill: #1f2328; font: 700 15px Avenir, "Segoe UI", Arial, sans-serif; }
.label { fill: #1f2328; font: 13px Avenir, "Segoe UI", Arial, sans-serif; }
.small { fill: #59636e; font: 11.5px Avenir, "Segoe UI", Arial, sans-serif; }
.tiny { fill: #59636e; font: 10.5px Avenir, "Segoe UI", Arial, sans-serif; }
.center { text-anchor: middle; }
.end { text-anchor: end; }
.axis { fill: none; stroke: #66717d; stroke-width: 1.5; }
.grid { fill: none; stroke: #d0d7de; stroke-width: 1; }
.arrow { fill: none; stroke: #66717d; stroke-width: 2; marker-end: url(#arrow); }
.blue { fill: #dbeafe; stroke: #1d6ff2; stroke-width: 1.5; }
.green { fill: #dcfce7; stroke: #238636; stroke-width: 1.5; }
.gold { fill: #fff3cd; stroke: #b77900; stroke-width: 1.5; }
.red { fill: #ffe4e0; stroke: #c2412d; stroke-width: 1.5; }
.violet { fill: #eee5ff; stroke: #8250df; stroke-width: 1.5; }
.blue-line { fill: none; stroke: #1d6ff2; stroke-width: 3; }
.green-line { fill: none; stroke: #238636; stroke-width: 3; }
.gold-line { fill: none; stroke: #b77900; stroke-width: 3; }
.red-line { fill: none; stroke: #c2412d; stroke-width: 3; }
.muted-line { fill: none; stroke: #8c959f; stroke-width: 1.5; stroke-dasharray: 5 4; }
.table-head { fill: #eaeef2; stroke: #afb8c1; stroke-width: 1; }
.table-cell { fill: #ffffff; stroke: #d0d7de; stroke-width: 1; }
@media (prefers-color-scheme: dark) {
  .background { fill: #0d1117; }
  .panel, .panel-dashed { fill: #161b22; stroke: #30363d; }
  .title, .heading, .label { fill: #e6edf3; }
  .subtitle, .small, .tiny { fill: #9da7b3; }
  .axis, .arrow { stroke: #8b949e; }
  .grid { stroke: #30363d; }
  .blue { fill: #15243a; stroke: #58a6ff; }
  .green { fill: #173326; stroke: #3fb950; }
  .gold { fill: #352b18; stroke: #d29922; }
  .red { fill: #3a211d; stroke: #f47067; }
  .violet { fill: #2d2340; stroke: #a371f7; }
  .table-head { fill: #21262d; stroke: #484f58; }
  .table-cell { fill: #161b22; stroke: #30363d; }
}
""".strip()


def root(title: str, description: str, height: int = 440) -> ET.Element:
    """Create a styled, accessible SVG root."""

    ET.register_namespace("", "http://www.w3.org/2000/svg")
    svg = ET.Element(
        "svg",
        {
            "xmlns": "http://www.w3.org/2000/svg",
            "viewBox": f"0 0 1000 {height}",
            "role": "img",
            "aria-labelledby": "title desc",
        },
    )
    ET.SubElement(svg, "title", {"id": "title"}).text = title
    description_element = ET.SubElement(svg, "desc", {"id": "desc"})
    description_element.text = "\n" + "\n".join(textwrap.wrap(description, width=80)) + "\n  "
    defs = ET.SubElement(svg, "defs")
    marker = ET.SubElement(
        defs,
        "marker",
        {"id": "arrow", "markerWidth": "9", "markerHeight": "7", "refX": "8", "refY": "3.5"},
    )
    ET.SubElement(marker, "polygon", {"points": "0 0, 9 3.5, 0 7", "fill": "#66717d"})
    ET.SubElement(defs, "style").text = STYLE
    ET.SubElement(svg, "rect", {"class": "background", "width": "1000", "height": str(height)})
    label(svg, title, 30, 38, "title")
    multiline(svg, description, 30, 64, 80, "subtitle", 15)
    return svg


def label(parent: ET.Element, value: str, x: float, y: float, css: str = "label") -> ET.Element:
    """Add one SVG text label."""

    element = ET.SubElement(parent, "text", {"class": css, "x": f"{x:g}", "y": f"{y:g}"})
    element.text = value
    return element


def multiline(
    parent: ET.Element,
    value: str,
    x: float,
    y: float,
    width: int,
    css: str = "small",
    gap: int = 15,
) -> ET.Element:
    """Add wrapped SVG text."""

    element = ET.SubElement(parent, "text", {"class": css, "x": f"{x:g}", "y": f"{y:g}"})
    for index, line in enumerate(textwrap.wrap(value, width=width)):
        attributes = {"x": f"{x:g}"}
        if index:
            attributes["dy"] = str(gap)
        ET.SubElement(element, "tspan", attributes).text = line
    return element


def box(
    parent: ET.Element,
    x: float,
    y: float,
    width: float,
    height: float,
    css: str = "panel",
    radius: int = 6,
) -> ET.Element:
    """Add a rectangular panel or process node."""

    return ET.SubElement(
        parent,
        "rect",
        {
            "class": css,
            "x": f"{x:g}",
            "y": f"{y:g}",
            "width": f"{width:g}",
            "height": f"{height:g}",
            "rx": str(radius),
        },
    )


def arrow(parent: ET.Element, x1: float, y1: float, x2: float, y2: float) -> ET.Element:
    """Add a straight arrow."""

    return ET.SubElement(
        parent,
        "line",
        {"class": "arrow", "x1": f"{x1:g}", "y1": f"{y1:g}", "x2": f"{x2:g}", "y2": f"{y2:g}"},
    )


def path(parent: ET.Element, data: str, css: str) -> ET.Element:
    """Add a styled SVG path."""

    return ET.SubElement(parent, "path", {"class": css, "d": data})


def polar_point(cx: float, cy: float, radius: float, degrees: float) -> tuple[float, float]:
    """Return a point where zero degrees is the upward axis."""

    angle = math.radians(degrees)
    return cx + radius * math.sin(angle), cy - radius * math.cos(angle)


def angular_acceptance() -> str:
    """Render the relationship between theta and delta_theta for uniform sampling."""

    svg = root(
        "Polar-angle acceptance",
        "Uniform theta sampling uses delta_theta as a half-width around the nominal direction.",
    )
    box(svg, 30, 86, 560, 324)
    box(svg, 610, 86, 360, 324)
    cx, cy, radius = 285, 360, 230
    lower, center, upper = 28, 40, 52
    low = polar_point(cx, cy, radius, lower)
    mid = polar_point(cx, cy, radius, center)
    high = polar_point(cx, cy, radius, upper)
    sector = (
        f"M {cx} {cy} L {low[0]:.1f} {low[1]:.1f} "
        f"A {radius} {radius} 0 0 1 {high[0]:.1f} {high[1]:.1f} Z"
    )
    path(svg, sector, "blue")
    ET.SubElement(svg, "line", {"class": "axis", "x1": str(cx), "y1": str(cy), "x2": str(cx), "y2": "105"})
    ET.SubElement(svg, "line", {"class": "axis", "x1": str(cx), "y1": str(cy), "x2": "555", "y2": str(cy)})
    for point, css in ((low, "muted-line"), (mid, "red-line"), (high, "muted-line")):
        ET.SubElement(
            svg,
            "line",
            {"class": css, "x1": str(cx), "y1": str(cy), "x2": f"{point[0]:.1f}", "y2": f"{point[1]:.1f}"},
        )
    label(svg, "beam axis", 268, 102, "small end")
    label(svg, "theta = 40 deg", mid[0] + 12, mid[1] - 8, "heading")
    label(svg, "28 deg", low[0] - 8, low[1] - 10, "small end")
    label(svg, "52 deg", high[0] + 8, high[1] + 5, "small")
    label(svg, "delta_theta", 355, 190, "small")
    label(svg, "delta_theta", 410, 225, "small")
    label(svg, "accepted sector: 24 deg = 2 * delta_theta", 310, 397, "small center")

    label(svg, "Example configuration", 635, 120, "heading")
    label(svg, "theta", 635, 154, "label")
    label(svg, "40 deg", 935, 154, "label end")
    label(svg, "delta_theta", 635, 184, "label")
    label(svg, "12 deg", 935, 184, "label end")
    label(svg, "randomThetaModel", 635, 214, "label")
    label(svg, "uniform", 935, 214, "label end")
    ET.SubElement(svg, "line", {"class": "grid", "x1": "635", "y1": "230", "x2": "945", "y2": "230"})
    multiline(svg, "Samples are uniform from theta - delta_theta through theta + delta_theta.", 635, 258, 43)
    multiline(
        svg,
        "For gaussian sampling, delta_theta is sigma and is not a hard angular boundary.",
        635,
        322,
        43,
    )
    return finish(svg)


def particle_models() -> str:
    """Render particle momentum, angle, and vertex randomization models."""

    svg = root(
        "Particle phase-space models",
        "The same center and delta parameters have model-dependent sampling interpretations.",
    )
    panels = ((30, "Uniform", "flat in center +/- delta", "blue"),
              (270, "Gaussian", "delta is one sigma", "red"),
              (510, "Cosine theta", "uniform in cos(theta)", "gold"),
              (750, "Vertex", "box, Gaussian, or sphere", "green"))
    for x, title, detail, _ in panels:
        box(svg, x, 92, 220, 305)
        label(svg, title, x + 110, 126, "heading center")
        label(svg, detail, x + 110, 148, "tiny center")
        ET.SubElement(
            svg,
            "line",
            {"class": "axis", "x1": str(x + 28), "y1": "330", "x2": str(x + 198), "y2": "330"},
        )
        ET.SubElement(
            svg,
            "line",
            {"class": "axis", "x1": str(x + 40), "y1": "175", "x2": str(x + 40), "y2": "340"},
        )

    path(svg, "M 72 330 L 72 230 L 208 230 L 208 330 Z", "blue")
    label(svg, "center-delta", 70, 354, "tiny")
    label(svg, "center+delta", 220, 354, "tiny end")
    path(svg, "M 310 330 C 350 328 355 190 380 185 C 405 190 410 328 452 330", "red-line")
    label(svg, "-sigma", 338, 354, "tiny center")
    label(svg, "mean", 380, 354, "tiny center")
    label(svg, "+sigma", 422, 354, "tiny center")
    path(svg, "M 550 330 C 575 325 595 275 620 215 C 640 177 665 220 692 330", "gold-line")
    label(svg, "theta min", 555, 354, "tiny")
    label(svg, "theta max", 700, 354, "tiny end")
    box(svg, 790, 205, 72, 72, "blue", 2)
    ET.SubElement(svg, "circle", {"class": "green", "cx": "900", "cy": "241", "r": "40"})
    ET.SubElement(svg, "circle", {"class": "red", "cx": "826", "cy": "241", "r": "5"})
    ET.SubElement(svg, "circle", {"class": "red", "cx": "900", "cy": "241", "r": "5"})
    label(svg, "component deltas", 826, 292, "tiny center")
    label(svg, "sphere radius", 900, 318, "tiny center")
    label(svg, "p, theta, phi", 140, 382, "small center")
    label(svg, "p, theta, vertex", 380, 382, "small center")
    label(svg, "theta only", 620, 382, "small center")
    label(svg, "vx, vy, vz", 860, 382, "small center")
    return finish(svg)


def event_lifecycle() -> str:
    """Render run setup and the processing stages inside each event."""

    svg = root(
        "What happens inside an allocated event",
        "Run-dependent setup happens once, then Geant4 and GEMC process each event through a fixed lifecycle.",
        470,
    )
    box(svg, 30, 90, 940, 72, "gold")
    label(svg, "Once per allocated run", 52, 119, "heading")
    label(svg, "set run -> constants -> translation table -> /run/beamOn N", 52, 145, "label")
    box(svg, 30, 184, 940, 244, "panel-dashed")
    label(svg, "Repeated for every event", 50, 213, "heading")
    stages = (
        (55, "Primary generator", "sample particles and create vertices", "blue"),
        (238, "Geant4 transport", "tracks, processes, and detector steps", "violet"),
        (421, "Sensitive detector", "key and accumulate steps into GHit", "red"),
        (604, "Digitization", "apply constants, response, and electronics", "gold"),
        (787, "Streamer", "publish headers, truth, and digitized banks", "green"),
    )
    for index, (x, title, detail, css) in enumerate(stages):
        box(svg, x, 245, 145, 128, css)
        label(svg, str(index + 1), x + 18, 269, "heading")
        label(svg, title, x + 72.5, 298, "heading center")
        multiline(svg, detail, x + 72.5, 323, 20, "small center", 15)
        if index < len(stages) - 1:
            arrow(svg, x + 149, 309, x + 178, 309)
    path(svg, "M 860 386 C 860 420 125 420 125 385", "arrow")
    label(svg, "next event until this run's assigned count is complete", 500, 454, "small center")
    return finish(svg)


def event_weights() -> str:
    """Render a run-weight input table, cumulative intervals, and expected counts."""

    svg = root(
        "Run-weight sampling example",
        "Relative weights define cumulative random intervals; integer event counts fluctuate stochastically.",
        450,
    )
    box(svg, 30, 90, 390, 310)
    label(svg, "weights.txt", 52, 120, "heading")
    columns = (52, 155, 270, 390)
    headers = ("Run", "Weight", "Normalized", "Expected")
    for index, header in enumerate(headers):
        label(svg, header, columns[index], 153, "small" if index < 3 else "small end")
    rows = (("11", "1", "0.10", "10"), ("12", "7", "0.70", "70"), ("13", "2", "0.20", "20"))
    for row_index, row in enumerate(rows):
        y = 184 + row_index * 44
        box(svg, 48, y - 23, 350, 34, "table-cell", 0)
        for index, value in enumerate(row):
            css = "label" if index < 3 else "label end"
            label(svg, value, columns[index], y, css)
    label(svg, "total", 52, 329, "heading")
    label(svg, "10", 155, 329, "heading")
    label(svg, "1.00", 270, 329, "heading")
    label(svg, "100", 390, 329, "heading end")
    multiline(svg, "Expected counts describe the long-run ratio, not a guaranteed allocation.", 52, 363, 48)

    box(svg, 445, 90, 525, 310)
    label(svg, "Cumulative selection intervals", 470, 120, "heading")
    label(svg, "draw = U(0, total weight)", 945, 120, "small end")
    segments = ((470, 47, "blue", "run 11", "0-1"),
                (517, 329, "gold", "run 12", "1-8"),
                (846, 94, "green", "run 13", "8-10"))
    for x, width, css, run, interval in segments:
        box(svg, x, 157, width, 78, css, 2)
        label(svg, run, x + width / 2, 188, "heading center")
        label(svg, interval, x + width / 2, 215, "small center")
    label(svg, "0", 470, 254, "small center")
    label(svg, "1", 517, 254, "small center")
    label(svg, "8", 846, 254, "small center")
    label(svg, "10", 940, 254, "small center")
    arrow(svg, 493, 278, 493, 316)
    arrow(svg, 681, 278, 681, 316)
    arrow(svg, 893, 278, 893, 316)
    label(svg, "about 10%", 493, 345, "small center")
    label(svg, "about 70%", 681, 345, "small center")
    label(svg, "about 20%", 893, 345, "small center")
    label(svg, "Every draw increments exactly one run counter.", 707, 380, "label center")
    return finish(svg)


def hit_scene() -> str:
    """Render tracks and accumulated steps in a segmented detector."""

    svg = root(
        "From detector steps to hits",
        "Steps sharing the same detector-cell key accumulate into one GHit; neighboring cells stay separate.",
        460,
    )
    box(svg, 30, 88, 610, 330)
    label(svg, "Segmented sensitive detector", 52, 119, "heading")
    cell_x, cell_y, size = 70, 145, 150
    for row in range(2):
        for column in range(3):
            css = "blue" if (row, column) == (0, 1) else "panel"
            box(svg, cell_x + column * size, cell_y + row * 120, size, 120, css, 0)
            label(svg, f"cell {row},{column}", cell_x + column * size + 10, cell_y + row * 120 + 22, "tiny")
    path(svg, "M 48 365 C 140 330 160 250 245 220 C 330 190 405 170 585 135", "red-line")
    path(svg, "M 55 205 C 170 205 250 210 360 245 C 455 275 520 330 610 365", "green-line")
    red_steps = ((250, 218, 5), (290, 202, 7), (335, 188, 9), (380, 177, 6))
    green_steps = ((245, 214, 5), (285, 220, 6), (330, 232, 8), (370, 249, 5))
    for x, y, radius in red_steps:
        ET.SubElement(svg, "circle", {"class": "red", "cx": str(x), "cy": str(y), "r": str(radius)})
    for x, y, radius in green_steps:
        ET.SubElement(svg, "circle", {"class": "green", "cx": str(x), "cy": str(y), "r": str(radius)})
    label(svg, "same cell identity", 295, 300, "heading center")
    label(svg, "two track discriminators -> two GHit objects", 295, 322, "small center")

    box(svg, 665, 88, 305, 330)
    label(svg, "What each dot contributes", 690, 120, "heading")
    legends = (("red", "energy deposit", "dot size can represent edep"),
               ("green", "track metadata", "track, parent, PDG, process"),
               ("blue", "position and time", "global/local xyz and global time"))
    for index, (css, title, detail) in enumerate(legends):
        y = 160 + index * 72
        ET.SubElement(svg, "circle", {"class": css, "cx": "704", "cy": str(y - 4), "r": "9"})
        label(svg, title, 725, y, "label")
        label(svg, detail, 725, y + 20, "tiny")
    box(svg, 690, 358, 250, 38, "violet")
    label(svg, "GHit stores aligned per-step vectors", 815, 383, "small center")
    return finish(svg)


def hit_accumulation() -> str:
    """Render an example step table and values derived from one hit."""

    svg = root(
        "GHit accumulation example",
        "One cell-keyed hit retains aligned step vectors and derives summary quantities lazily.",
        450,
    )
    box(svg, 30, 90, 525, 315)
    label(svg, "Stored step vectors", 52, 122, "heading")
    headers = ((52, "Step"), (130, "edep"), (225, "time"), (320, "local x"), (430, "track"))
    for x, value in headers:
        label(svg, value, x, 156, "small")
    rows = (("1", "0.12 MeV", "5.1 ns", "-1.4 mm", "42"),
            ("2", "0.35 MeV", "5.4 ns", "-0.8 mm", "42"),
            ("3", "0.08 MeV", "5.9 ns", "+0.1 mm", "42"),
            ("4", "0.21 MeV", "6.2 ns", "+0.9 mm", "42"))
    for row_index, row in enumerate(rows):
        y = 190 + row_index * 43
        box(svg, 48, y - 23, 480, 34, "table-cell", 0)
        for (x, _), value in zip(headers, row):
            label(svg, value, x, y, "label")
    label(svg, "vectors remain index-aligned across all stored quantities", 52, 382, "small")
    arrow(svg, 565, 247, 615, 247)
    box(svg, 630, 90, 340, 315)
    label(svg, "Derived hit view", 654, 122, "heading")
    summaries = (("Total edep", "0.76 MeV", "red"),
                 ("Time extent", "5.1-6.2 ns", "gold"),
                 ("Representative track", "track 42", "green"),
                 ("Step count", "4", "blue"))
    for index, (name, value, css) in enumerate(summaries):
        y = 146 + index * 58
        box(svg, 654, y, 292, 44, css)
        label(svg, name, 672, y + 27, "label")
        label(svg, value, 928, y + 27, "heading end")
    multiline(svg, "Digitizers can consume both raw vectors and derived values.", 654, 384, 42)
    return finish(svg)


def data_hierarchy() -> str:
    """Render event, run, and frame aggregation paths in gdata."""

    svg = root(
        "GData aggregation scopes",
        "Hit-side objects follow event and run aggregation paths; frames model time-window payloads "
        "separately.",
        455,
    )
    label(svg, "Event and run path", 40, 108, "heading")
    stages = ((40, 135, 150, "GTrueInfoData", "truth per hit", "blue"),
              (40, 205, 150, "GDigitizedData", "response per hit", "gold"),
              (245, 170, 165, "GDataCollection", "one detector", "violet"),
              (465, 170, 180, "GEventDataCollection", "many detectors", "red"),
              (710, 170, 180, "GRunDataCollection", "integrated events", "green"))
    for x, y, width, title, detail, css in stages:
        box(svg, x, y, width, 58, css)
        label(svg, title, x + width / 2, y + 25, "heading center")
        label(svg, detail, x + width / 2, y + 45, "tiny center")
    arrow(svg, 190, 164, 240, 190)
    arrow(svg, 190, 234, 240, 204)
    arrow(svg, 414, 199, 460, 199)
    arrow(svg, 650, 199, 705, 199)
    label(svg, "append hit entries", 327, 258, "small center")
    label(svg, "group by detector", 555, 258, "small center")
    label(svg, "additive integration", 800, 258, "small center")

    box(svg, 40, 310, 850, 100, "panel-dashed")
    label(svg, "Frame / streaming path", 62, 341, "heading")
    box(svg, 245, 332, 170, 55, "blue")
    label(svg, "GFrameHeader", 330, 365, "heading center")
    box(svg, 470, 332, 170, 55, "gold")
    label(svg, "GIntegralPayload", 555, 365, "heading center")
    arrow(svg, 420, 359, 465, 359)
    box(svg, 700, 332, 170, 55, "green")
    label(svg, "GFrameDataCollection", 785, 365, "heading center")
    arrow(svg, 645, 359, 695, 359)
    label(svg, "time-window grouping, not an event hierarchy", 455, 437, "small center")
    return finish(svg)


def options_precedence() -> str:
    """Render option source precedence and structured dot-notation updates."""

    svg = root(
        "Configuration precedence and updates",
        "Defaults establish a complete schema, YAML supplies a baseline, and command-line values override it.",
        440,
    )
    layers = ((70, 290, 310, 78, "blue", "Schema defaults", "complete missing optional keys"),
              (110, 215, 310, 78, "gold", "YAML configuration", "reproducible file baseline"),
              (150, 140, 310, 78, "red", "Command line", "highest-precedence updates"))
    for x, y, width, height, css, title, detail in layers:
        box(svg, x, y, width, height, css)
        label(svg, title, x + 22, y + 31, "heading")
        label(svg, detail, x + 22, y + 56, "small")
    arrow(svg, 475, 255, 560, 255)
    box(svg, 580, 112, 360, 270, "panel")
    label(svg, "Resolved option tree", 605, 145, "heading")
    entries = (("runno", "12", "gold"),
               ("verbosity.gemc", "2", "red"),
               ("gparticle[0].name", "e-", "gold"),
               ("gparticle[0].p", "5*GeV", "red"))
    for index, (name, value, css) in enumerate(entries):
        y = 166 + index * 46
        box(svg, 605, y, 310, 36, css, 3)
        label(svg, name, 620, y + 24, "small")
        label(svg, value, 895, y + 24, "label end")
    label(svg, "validation", 605, 365, "small")
    label(svg, "types + mandatory keys + snapshot", 915, 365, "small end")
    return finish(svg)


def field_map() -> str:
    """Render a sampled magnetic field and its attachment to a detector volume."""

    svg = root(
        "Field evaluation and volume attachment",
        "A named field plugin evaluates B at each point while its field manager controls Geant4 integration.",
        450,
    )
    box(svg, 30, 90, 555, 315)
    label(svg, "Sampled field map", 52, 122, "heading")
    box(svg, 70, 145, 465, 215, "panel-dashed", 2)
    for x in range(105, 510, 65):
        ET.SubElement(svg, "line", {"class": "grid", "x1": str(x), "y1": "155", "x2": str(x), "y2": "350"})
    for y in range(170, 350, 45):
        ET.SubElement(svg, "line", {"class": "grid", "x1": "80", "y1": str(y), "x2": "525", "y2": str(y)})
    for row, y in enumerate(range(180, 340, 50)):
        for column, x in enumerate(range(100, 510, 68)):
            offset = (column - 2.5) * 2
            length = 22 + row * 4
            arrow(svg, x, y, x + offset, y - length)
    ET.SubElement(svg, "circle", {"class": "red", "cx": "305", "cy": "253", "r": "7"})
    label(svg, "configured origin", 318, 258, "small")
    label(svg, "B(x, y, z)", 92, 386, "label")
    label(svg, "arrow direction = field direction; length = magnitude", 548, 386, "small end")

    box(svg, 610, 90, 360, 315)
    label(svg, "Runtime attachment", 635, 122, "heading")
    nodes = ((635, 145, "GFieldDefinition", "name, plugin, strength", "blue"),
             (635, 207, "GField plugin", "evaluate field vector", "violet"),
             (635, 269, "G4FieldManager", "stepper + chord finder", "gold"),
             (635, 331, "Logical volume", "transport uses the field", "green"))
    for index, (x, y, title, detail, css) in enumerate(nodes):
        box(svg, x, y, 310, 47, css)
        label(svg, title, x + 16, y + 21, "heading")
        label(svg, detail, x + 294, y + 21, "tiny end")
        if index < len(nodes) - 1:
            arrow(svg, x + 155, y + 49, x + 155, y + 60)
    return finish(svg)


def streamer_publication() -> str:
    """Render event buffering and the fixed event publication hook sequence."""

    svg = root(
        "Buffered event publication",
        "The base streamer owns batching and hook order; format plugins implement serialization details.",
        465,
    )
    label(svg, "Event buffer", 38, 112, "heading")
    for index in range(4):
        x = 38 + index * 105
        box(svg, x, 132, 88, 58, "blue")
        label(svg, f"event {index + 1}", x + 44, 157, "heading center")
        label(svg, "shared data", x + 44, 178, "tiny center")
    arrow(svg, 470, 161, 535, 161)
    box(svg, 550, 124, 165, 74, "gold")
    label(svg, "flush", 632.5, 153, "heading center")
    label(svg, "limit or explicit call", 632.5, 177, "tiny center")
    arrow(svg, 720, 161, 790, 161)
    box(svg, 805, 124, 155, 74, "green")
    label(svg, "plugin output", 882.5, 153, "heading center")
    label(svg, "file / records", 882.5, 177, "tiny center")

    box(svg, 30, 226, 940, 190, "panel-dashed")
    label(svg, "Fixed event hook sequence", 50, 257, "heading")
    hooks = ((50, "startEvent", "red"),
             (205, "event header", "blue"),
             (360, "generated", "violet"),
             (515, "true info", "gold"),
             (670, "digitized", "green"),
             (825, "endEvent", "red"))
    for index, (x, title, css) in enumerate(hooks):
        box(svg, x, 286, 125, 60, css)
        label(svg, str(index + 1), x + 15, 310, "small")
        label(svg, title, x + 62.5, 328, "heading center")
        if index < len(hooks) - 1:
            arrow(svg, x + 129, 316, x + 150, 316)
    label(svg, "ROOT", 260, 390, "small center")
    label(svg, "CSV", 420, 390, "small center")
    label(svg, "JSON", 580, 390, "small center")
    label(svg, "ASCII", 740, 390, "small center")
    label(svg, "JLAB SRO", 880, 390, "small center")
    label(svg, "same semantic sequence, backend-specific representation", 570, 446, "small center")
    return finish(svg)


def finish(svg: ET.Element) -> str:
    """Serialize a diagram as formatted standalone SVG."""

    ET.indent(svg, space="  ")
    return '<?xml version="1.0" encoding="UTF-8"?>\n' + ET.tostring(svg, encoding="unicode") + "\n"


RENDERERS = {
    "eventdispenser-event-lifecycle.svg": event_lifecycle,
    "eventdispenser-weight-example.svg": event_weights,
    "gdata-hierarchy.svg": data_hierarchy,
    "gfields-field-map.svg": field_map,
    "ghit-accumulation.svg": hit_accumulation,
    "ghit-detector-steps.svg": hit_scene,
    "goptions-precedence.svg": options_precedence,
    "gparticle-angular-acceptance.svg": angular_acceptance,
    "gparticle-randomization-models.svg": particle_models,
    "gstreamer-publication.svg": streamer_publication,
}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="fail when checked-in diagrams are stale")
    args = parser.parse_args()

    stale: list[Path] = []
    OUTPUT_DIRECTORY.mkdir(parents=True, exist_ok=True)
    for filename, renderer in RENDERERS.items():
        destination = OUTPUT_DIRECTORY / filename
        content = renderer()
        if args.check:
            if not destination.exists() or destination.read_text(encoding="utf-8") != content:
                stale.append(destination)
        else:
            destination.write_text(content, encoding="utf-8")

    if stale:
        print("Doxygen concept diagrams are missing or stale:")
        for destination in stale:
            print(f"  {destination}")
        print("Run ci/generate_doxygen_concept_diagrams.py to regenerate them.")
        return 1

    action = "checked" if args.check else "generated"
    print(f"Doxygen concept diagrams {action}: {len(RENDERERS)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
