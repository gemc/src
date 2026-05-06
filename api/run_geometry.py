import importlib.util
import sys
import os
import pyvista as pv

def run_geometry(script, args=None, db="gemc.db"):
    if os.path.exists(db):
        os.remove(db)

    pv.set_jupyter_backend("trame")
    pv.global_theme.trame.default_mode = "local"

    sys.argv = [script] + (args or [])

    spec = importlib.util.spec_from_file_location("target", script)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    return module.cfg.show(block=False)