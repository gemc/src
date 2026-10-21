"""Check option discovery through the CLI, including fields omitted from summary help."""

from pathlib import Path
import re
import subprocess
import sys
import tempfile


def main():
    fixture, gemc = (str(Path(argument).resolve()) for argument in sys.argv[1:])
    with tempfile.TemporaryDirectory(prefix="gemc-search-") as workdir:
        def run(executable, *arguments):
            result = subprocess.run(
                [executable, *arguments], cwd=workdir, text=True,
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=15, check=True,
            )
            return re.sub(r"\x1b\[[0-9;]*m", "", result.stdout)

        cases = {
            "fixture_scalar": "fixture_scalar",       # option name
            "SCALAR SYNOPSIS": "fixture_scalar",      # summary, case insensitive
            "scalar extended guidance": "fixture_scalar",
            "SCALAR_INITIAL": "fixture_scalar",       # scalar default
            "schema_key": "fixture_schema",           # schema key
            "field annotation": "fixture_schema",     # key description
            "schema_initial": "fixture_schema",       # structured default
            "optional_key": "fixture_schema",         # null default
            "1739": "fixture_schema",                 # typed integer default
            "enabled_key: true": "fixture_schema",    # typed boolean default
            "required_key": "fixture_cumulative",     # required key
            "cumulative_initial": "fixture_cumulative",  # unset cumulative value
            "fixture_switch": "fixture_switch",
            "TOGGLE ANNOTATION": "fixture_switch",
        }
        for query, expected in cases.items():
            output = run(fixture, "search", query)
            entries = re.findall(r"^\s+-(\w+)(?:=|\s)", output, flags=re.MULTILINE)
            assert entries == [expected], (query, output)
            assert "Default value:" not in output, output
            assert "no match found" not in output, output

        assert "no match found" in run(fixture, "search", "no_such_search_term_83729")
        assert "schema_key: field annotation" in run(fixture, "help", "fixture_schema")
        assert "case-insensitive" in run(fixture, "-h")
        for query in ("driver", "OGL", "oGl", "cloudPoints", "cloud volume rendering", "1.0 1.0 1.0"):
            output = run(gemc, "search", query)
            assert len(re.findall(r"^\s+-g4view=", output, flags=re.MULTILINE)) == 1, (query, output)
        assert "OGL" in run(gemc, "help", "g4view")
        print("Option search checks passed (metadata fields and g4view discovery).")


if __name__ == "__main__":
    main()
