#!/usr/bin/env python3

import os

# Path to the directory containing subdirectories with Doxygen documentation
pages_directory = "pages"

# HTML template for the header
html_header = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Doxygen Documentation Links</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 0;
            background-color: #f4f4f9;
            color: #333;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
        }

        .container {
            text-align: center;
            padding: 120px;
            background-color: #ffffff;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            border-radius: 10px;
            width: 90%;
            max-width: 600px;
        }

        h1 {
            color: #4a90e2;
            margin-bottom: 10px;
        }

        p {
            color: #555;
            font-size: 1.1em;
            margin-bottom: 20px;
        }

        .link-list {
          display: flex;
            flex-wrap: wrap;
            justify-content: center;
            padding: 0;
            margin: 0;
            list-style: none;
        }

        .link-list li {
        margin: 10px;
            flex: 0 1 calc(50% - 20px);
            display: flex;
            justify-content: center;
        }

        .link-list a {
            text-decoration: none;
            color: #ffffff;
            background-color: #4a30a2;
            padding: 10px 20px;
            border-radius: 5px;
            transition: background-color 0.1s;
            display: inline-block;
            width: 100%;
            max-width: 300px;
            text-align: center;
        }

        .link-list a:hover {
            background-color: #4a70e2;
        }

        .footer {
            margin-top: 20px;
            font-size: 0.9em;
            color: #999;
        }
        
        @media (min-width: 768px) {
            .link-list {
                column-count: 2;
            }
        }
        
    </style>
</head>
<body>
    <div class="container">
        <h1><a href="https://gemc.github.io/home/">GEMC Home</a></h1>
        <h1>Doxygen Documentation</h1>
        <p>Explore the Doxygen documentation the various gemc classes.</p>
        <ul class="link-list">
"""

# HTML template for the footer
html_footer = """
        </ul>
        <div class="footer">
            © 2025 GEMC - Maurizio Ungaro
        </div>
    </div>
</body>
</html>
"""


# Generate the HTML content
def generate_html(directory):
    links = ""
    for subdir in os.listdir(directory):
        subdir_path = os.path.join(directory, subdir)
        if os.path.isdir(subdir_path):
            index_file = os.path.join(subdir_path, "index.html")
            if os.path.exists(index_file):
                # Update the link to be relative to the pages directory
                relative_index_file = os.path.relpath(index_file, directory)
                links += f'            <li><a href="{relative_index_file}" target="_blank">{subdir} Documentation</a></li>\n'
    return html_header + links + html_footer


# Write the HTML content to a file in the pages directory
def write_html_file(directory):
    html_content = generate_html(directory)
    output_file = os.path.join(directory, "index.html")
    with open(output_file, "w") as file:
        file.write(html_content)
    print(f"HTML file '{output_file}' has been generated.")


# Run the script
write_html_file(pages_directory)
