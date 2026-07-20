#!/usr/bin/env python3

"""Generate the top-level index for the per-module Doxygen sites."""

from __future__ import annotations

import html
import shutil
from pathlib import Path


MODULE_GROUPS = {
    "Core": ("guts", "goptions", "glogging", "gbase", "textProgressBar", "utilities"),
    "Extension points": ("gfactory", "gfields", "gstreamer"),
    "Digitization": ("gtouchable", "ghit", "gdynamicDigitization", "gsd"),
    "User interface": (
        "gboard",
        "gqtbuttonswidget",
        "g4display",
        "g4dialog",
        "gsplash",
        "gtree",
        "dbselect",
        "gui",
    ),
    "Detector construction": ("gsystem", "g4system", "gdetector", "gtranslationTable"),
    "Data and events": ("gdata", "eventDispenser"),
    "Geant4 runtime": ("gparticle", "gphysics", "actions"),
}

MODULE_DESCRIPTIONS = {
    "actions": "Geant4 run, event, stepping, tracking, and primary-generator actions.",
    "dbselect": "Qt detector-configuration selector and geometry reload workflow.",
    "eventDispenser": "Run-weighted event allocation and processing.",
    "g4dialog": "Qt browser and command prompt for the Geant4 UI.",
    "g4display": "Qt controls for Geant4 views, lighting, cuts, and scene decoration.",
    "g4system": "Construction of Geant4 solids, materials, logical volumes, and placements.",
    "gbase": "Shared base classes, configuration access, and logging conventions.",
    "gboard": "Qt log viewer and Geant4 output-session adapter.",
    "gdata": "Truth, digitized, event, run, and frame data containers.",
    "gdetector": "Detector-construction bridge joining systems, fields, and sensitive detectors.",
    "gdynamicDigitization": "Runtime-loaded detector digitization interfaces and readout rules.",
    "gfactory": "Static and dynamic factory registration and plugin loading.",
    "gfields": "Magnetic-field configuration, factories, and Geant4 field managers.",
    "ghit": "Per-step and aggregated simulation-hit information.",
    "glogging": "Structured logging, verbosity, debug levels, and message formatting.",
    "goptions": "YAML and command-line option definition, parsing, and validation.",
    "gparticle": "Particle-source definitions, file input, and generated-particle records.",
    "gphysics": "Geant4 physics-list selection and construction.",
    "gqtbuttonswidget": "Reusable Qt icon strips and toggle-button groups.",
    "gsd": "Thread-local Geant4 sensitive detector and hit processing.",
    "gsplash": "Optional Qt splash-screen lifecycle and status messages.",
    "gstreamer": "Plugin-based ASCII, CSV, JSON, ROOT, and SRO output.",
    "gsystem": "Detector systems, materials, geometry factories, and world assembly.",
    "gtouchable": "Stable detector-element identities and hit-merging discriminators.",
    "gtranslationTable": "Mappings from detector identities to electronics channels.",
    "gtree": "Interactive Qt geometry tree, styling, and volume inspection.",
    "gui": "Top-level GEMC Qt interface and simulation controls.",
    "guts": "Dependency-light string, path, parsing, and console helpers.",
    "textProgressBar": "Terminal progress reporting for long-running loops.",
    "utilities": "Geant4 runtime, signal, random-seed, and resource helpers.",
}

PAGES_DIRECTORY = Path("pages")
ASSET_SOURCE = Path(__file__).with_name("doxygen-images") / "gemc-architecture.svg"

HTML_HEADER = """<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>GEMC C++ API Documentation</title>
  <style>
    :root {
      color-scheme: light dark;
      --bg: #ffffff;
      --fg: #1f2328;
      --muted: #59636e;
      --border: #d0d7de;
      --link: #0969da;
      --panel: #f6f8fa;
      --accent: #1f883d;
    }

    @media (prefers-color-scheme: dark) {
      :root {
        --bg: #0d1117;
        --fg: #e6edf3;
        --muted: #9da7b3;
        --border: #30363d;
        --link: #58a6ff;
        --panel: #161b22;
        --accent: #3fb950;
      }
    }

    * { box-sizing: border-box; }

    body {
      margin: 0;
      background: var(--bg);
      color: var(--fg);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Helvetica, Arial, sans-serif;
      font-size: 16px;
      line-height: 1.5;
    }

    main, footer {
      width: min(1120px, calc(100% - 40px));
      margin: 0 auto;
    }

    header { padding: 36px 0 18px; }

    h1 {
      margin: 0 0 8px;
      font-size: 2rem;
      font-weight: 650;
      letter-spacing: 0;
    }

    header p, footer p {
      max-width: 760px;
      margin: 0;
      color: var(--muted);
    }

    .architecture {
      margin: 18px 0 34px;
      padding: 16px;
      background: var(--panel);
      border: 1px solid var(--border);
      border-radius: 6px;
      text-align: center;
    }

    .architecture img {
      display: block;
      width: min(100%, 820px);
      max-height: 520px;
      margin: 0 auto;
    }

    .architecture figcaption {
      margin-top: 10px;
      color: var(--muted);
      font-size: 0.9rem;
    }

    .section {
      margin: 0 0 30px;
      border-top: 2px solid var(--accent);
    }

    .section h2 {
      margin: 10px 0 4px;
      font-size: 1.25rem;
      letter-spacing: 0;
    }

    .module-list {
      display: grid;
      grid-template-columns: repeat(2, minmax(0, 1fr));
      margin: 10px 0 0;
      padding: 0;
      list-style: none;
      border-bottom: 1px solid var(--border);
    }

    .module-list li { border-top: 1px solid var(--border); }
    .module-list li:nth-child(odd) { border-right: 1px solid var(--border); }

    .module-list a {
      display: block;
      min-height: 88px;
      padding: 14px 16px;
      color: inherit;
      text-decoration: none;
    }

    .module-list a:hover, .module-list a:focus-visible { background: var(--panel); }

    .module-name {
      display: block;
      margin-bottom: 4px;
      color: var(--link);
      font-family: ui-monospace, SFMono-Regular, Menlo, Consolas, monospace;
      font-weight: 650;
    }

    .module-description { color: var(--muted); }

    footer {
      margin-top: 8px;
      padding: 22px 0 40px;
      border-top: 1px solid var(--border);
    }

    footer a { color: var(--link); }

    @media (max-width: 720px) {
      main, footer { width: min(100% - 24px, 1120px); }
      header { padding-top: 24px; }
      .architecture { padding: 8px; }
      .module-list { grid-template-columns: minmax(0, 1fr); }
      .module-list li:nth-child(odd) { border-right: 0; }
    }
  </style>
</head>
<body>
  <main>
    <header>
      <h1>GEMC C++ API Documentation</h1>
      <p>Browse the implementation, public interfaces, configuration, and examples for each GEMC module.</p>
    </header>
    <figure class="architecture">
      <img src="assets/gemc-architecture.svg" alt="GEMC simulation data flow">
      <figcaption>
        Detector definitions become Geant4 geometry, hits, digitized data, and output streams.
      </figcaption>
    </figure>
"""

HTML_FOOTER = """  </main>
  <footer>
    <p>
      GEMC project documentation.
      <a href="https://gemc.github.io/home/documentation">Documentation home</a>
    </p>
  </footer>
</body>
</html>
"""


def existing_doc_dirs(directory: Path) -> dict[str, Path]:
    """Return module names and relative index paths for generated module sites."""

    return {
        path.name: path.joinpath("index.html").relative_to(directory)
        for path in sorted(directory.iterdir(), key=lambda entry: entry.name.casefold())
        if path.is_dir() and path.joinpath("index.html").is_file()
    }


def module_list(modules: list[str], existing: dict[str, Path]) -> str:
    """Render one module link list."""

    items = []
    for module in modules:
        href = html.escape(existing[module].as_posix(), quote=True)
        label = html.escape(module)
        description = html.escape(MODULE_DESCRIPTIONS.get(module, "Module API documentation."))
        items.append(
            "      <li>"
            f'<a href="{href}"><span class="module-name">{label}</span>'
            f'<span class="module-description">{description}</span></a></li>\n'
        )
    return '    <ul class="module-list">\n' + "".join(items) + "    </ul>\n"


def generate_html(directory: Path) -> str:
    """Generate the landing page for all module sites found below directory."""

    existing = existing_doc_dirs(directory)
    used: set[str] = set()
    sections = []

    for section_name, configured_modules in MODULE_GROUPS.items():
        present = [module for module in configured_modules if module in existing]
        if not present:
            continue
        used.update(present)
        sections.append(
            '    <section class="section">\n'
            f"      <h2>{html.escape(section_name)}</h2>\n"
            f"{module_list(present, existing)}"
            "    </section>\n"
        )

    leftovers = sorted(set(existing) - used, key=str.casefold)
    if leftovers:
        sections.append(
            '    <section class="section">\n'
            "      <h2>Other modules</h2>\n"
            f"{module_list(leftovers, existing)}"
            "    </section>\n"
        )

    return HTML_HEADER + "".join(sections) + HTML_FOOTER


def write_html_file(directory: Path) -> None:
    """Write the landing page and its self-contained architecture asset."""

    assets_directory = directory / "assets"
    assets_directory.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(ASSET_SOURCE, assets_directory / ASSET_SOURCE.name)

    output_file = directory / "index.html"
    output_file.write_text(generate_html(directory), encoding="utf-8")
    print(f"HTML file '{output_file}' has been generated.")


if __name__ == "__main__":
    write_html_file(PAGES_DIRECTORY)
