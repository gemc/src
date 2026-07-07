#!/usr/bin/env python3
"""Upload the CAD "organs" example into gemc.db.

All geometry is authored declaratively in stls/cad__default.yaml (mesh name, scale, position,
rotation, color, material, dosimeter sensitivity). This script only uploads that definition into
the SQLite database with the pygemc CAD feature; it performs no mesh scaling or positioning.

Run it with the pygemc python from this directory:

    /opt/projects/gemc/src/build/subprojects/pygemc/python_env/bin/python cad.py

Then run the example (gamma source, per-organ dose) with:

    /opt/projects/gemc/src/build/bin/gemc cad.yaml -n=10
"""

import os

from pygemc.api.gcad import upload_cad_definitions

HERE = os.path.dirname(os.path.abspath(__file__))


def main():
    cad_file = os.path.join(HERE, "stls", "cad__default.yaml")
    db_path = os.path.join(HERE, "gemc.db")
    upload_cad_definitions(cad_file, db_path, experiment="examples")


if __name__ == "__main__":
    main()
