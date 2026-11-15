"""A real production event action must publish the active collection after an inactive slot."""
import json
from pathlib import Path
import subprocess
import sys
import os
import tempfile

for argument in sys.argv[2:]:
    if argument.startswith("--plugin-path="):
        os.environ["GEMC_PLUGIN_PATH"] = argument.split("=", 1)[1]

binary=str(Path(sys.argv[1]).resolve());source=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory() as directory:
    work=Path(directory)
    database=work/'geometry.db'
    setup=subprocess.run([sys.executable,str(source/'examples/basic/simple_flux/simple_flux.py'),'-f','sqlite','-sql',str(database)],cwd=work,capture_output=True,text=True)
    assert setup.returncode==0,setup.stdout+setup.stderr
    run=subprocess.run([binary,'-gsystem=[{name: simple_flux, factory: sqlite}]','-experiment=examples','-runno=1','-sql='+str(database),'-nthreads=1','-recordZeroEdep','-gparticle=[{name: geantino, p: 1*GeV, vz: -3*cm}]','-gstreamer=[{format: json, filename: sparse}]'],cwd=work,capture_output=True,text=True)
    assert run.returncode==0,run.stdout+run.stderr
    events=json.loads((work/'sparse_t0.json').read_text())['events']
    assert len(events)==1,events
    assert events[0].get('detectors',{}).get('digitized_by_detector',{}).get('flux'),events
print('Production event dispatch preserves hits at collection ID 1 with inactive ID 0.')
