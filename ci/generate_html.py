#!/usr/bin/env python3

import os
import html

modules_map = {
    "base": "guts goptions glogging gbase textProgressBar utilities",
    "plugins": "gfactory gdynamicDigitization gsd gfields",
    "sensitivity": "gtouchable ghit",
    "gui": "gboard gqtbuttonswidget g4display g4dialog gsplash gtree dbselect gui",
    "detector": "gsystem g4system gdetector gtranslationTable",
    "i/o": "gdata gstreamer eventDispenser",
    "geant4": "gparticle gphysics  actions",
}

# Path to the directory containing subdirectories with Doxygen documentation
pages_directory = "pages"

# Number of columns to show in the link grid (set to any integer >= 1)
num_columns = 5

# HTML template for the header
html_header = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Doxygen Documentation Links</title>
    <style>
        :root {
            --fg: #24292f;
            --muted: #57606a;
            --border: #d0d7de;
            --link: #0969da;
            --bg: #ffffff;
        }

        body {
            margin: 0;
            background: var(--bg);
            color: var(--fg);
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", "Noto Sans", Helvetica, Arial, sans-serif, "Apple Color Emoji", "Segoe UI Emoji";
        }

        /* GitHub-ish readable content width */
        .container {
            max-width: 980px;
            margin: 0 auto;
            padding: 32px 24px 24px 24px;
        }

        /* Intro line like a README paragraph */
        p {
            margin: 0 0 16px 0;
            color: var(--muted);
            font-size: 16px;
            line-height: 1.5;
        }

        /* Outer list holds sections (footer expects one outer <ul>) */
        .link-list {
            list-style: none;
            padding: 0;
            margin: 16px 0 0 0;
        }

        /* Each section as a list item */
        .section {
            margin: 0 0 20px 0;
        }

        /* “Same font as ## but smaller” */
        .section-title {
            font-weight: 600;
            font-size: 18px;          /* smaller than typical H2 */
            line-height: 1.25;
            margin: 24px 0 10px 0;
            padding-bottom: 6px;
            border-bottom: 1px solid var(--border);
        }

        /* Links under each section */
        .section-links {
            list-style: none;
            padding: 0;
            margin: 10px 0 0 0;

            /* compact grid-ish layout without “button” styling */
            column-count: {{NUM_COLUMNS}};
            column-gap: 18px;
        }

        .section-links li {
            break-inside: avoid;
            margin: 6px 0;
        }

        a {
            color: var(--link);
            text-decoration: none;
        }

        a:hover {
            text-decoration: underline;
        }

        /* Keep footer styles as originally referenced */
        .footer {
            margin-top: 20px;
            font-size: 0.9em;
            color: #999;
        }
    </style>
</head>
<body>
    <div class="container">
        <p>Explore the Doxygen documentation the various gemc modules.</p> <br/><br/>
        <ul class="link-list">
"""

# HTML template for the footer (KEEP AS IS)
html_footer = """
        </ul>
        <div class="footer">
            <br/><br/>
            © 2026 GEMC - Maurizio Ungaro
            <br/><br/>
            <h3><a href="https://gemc.github.io/home/documentation">Back to GEMC Documentation</a></h3>
        </div>
    </div>
</body>
</html>
"""


def _existing_doc_dirs(directory: str) -> dict[str, str]:
    """
    Return {subdir_name: relative_index_file} for subdirectories that contain index.html.
    """
    out: dict[str, str] = {}
    for name in sorted(os.listdir(directory), key=str.casefold):
        subdir_path = os.path.join(directory, name)
        if not os.path.isdir(subdir_path):
            continue
        index_file = os.path.join(subdir_path, "index.html")
        if not os.path.exists(index_file):
            continue
        out[name] = os.path.relpath(index_file, directory)
    return out


def generate_html(directory: str, columns: int) -> str:
    """Generate the index.html content listing subdirectories organized by modules_map."""
    # Clamp columns to at least 1 (avoids division by zero / bad CSS)
    columns = max(1, int(columns))

    existing = _existing_doc_dirs(directory)
    used: set[str] = set()

    sections_html: list[str] = []

    # Build sections in insertion order of modules_map
    for section_name, modules_str in modules_map.items():
        modules = [m for m in modules_str.split() if m.strip()]
        # Only include modules that actually exist and have index.html
        present = [m for m in modules if m in existing]
        if not present:
            continue

        used.update(present)

        section_title = html.escape(section_name)
        section_block = [
            '            <li class="section">\n',
            f'                <div class="section-title">{section_title}</div>\n',
            '                <ul class="section-links">\n',
        ]

        for m in present:
            href = html.escape(existing[m], quote=True)
            label = html.escape(m)
            # Text must visibly include square brackets: [ link ]
            section_block.append(f'                    <li><a href="{href}" target="_blank">[ {label} ]</a></li>\n')

        section_block.append("                </ul>\n")
        section_block.append("            </li>\n")
        section_block.append("            <br/>\n")
        sections_html.append("".join(section_block))

    # Optional: any leftover documented dirs not in modules_map
    leftovers = sorted([k for k in existing.keys() if k not in used], key=str.casefold)
    if leftovers:
        section_block = [
            '            <li class="section">\n',
            '                <div class="section-title">other</div>\n',
            '                <ul class="section-links">\n',
        ]
        for m in leftovers:
            href = html.escape(existing[m], quote=True)
            label = html.escape(m)
            section_block.append(f'                    <li><a href="{href}" target="_blank">[ {label} ]</a></li>\n')
        section_block.append("                </ul>\n")
        section_block.append("            </li>\n")
        section_block.append("            <br/>\n")
        sections_html.append("".join(section_block))

    header = html_header.replace("{{NUM_COLUMNS}}", str(columns))
    return header + "".join(sections_html) + html_footer


def write_html_file(directory: str, columns: int) -> None:
    """Write the generated HTML to pages/index.html."""
    html_content = generate_html(directory, columns)
    output_file = os.path.join(directory, "index.html")

    with open(output_file, "w", encoding="utf-8") as file:
        file.write(html_content)

    print(f"HTML file '{output_file}' has been generated.")


if __name__ == "__main__":
    write_html_file(pages_directory, num_columns)
