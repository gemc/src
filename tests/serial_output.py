"""Serial and MT contexts retain generated events and integrated dose exactly once."""
import csv
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile

for argument in sys.argv[2:]:
    if argument.startswith("--plugin-path="):
        os.environ["GEMC_PLUGIN_PATH"] = argument.split("=", 1)[1]

binary = str(Path(sys.argv[1]).resolve())
source = Path(__file__).resolve().parents[1]
failures = []
with tempfile.TemporaryDirectory() as directory:
    database=Path(directory)/'geometry.db'
    setup=subprocess.run([sys.executable,str(source/'examples/basic/b1/b1.py'),'-f','sqlite','-sql',str(database)],cwd=directory,capture_output=True,text=True)
    assert setup.returncode==0,setup.stdout+setup.stderr
    for mode, threads in [('Serial',1),('MT',1),('MT',2)]:
        work = Path(directory)/f'{mode}_{threads}'
        work.mkdir()
        env = os.environ.copy(); env['G4RUN_MANAGER_TYPE'] = mode
        result = subprocess.run([binary,'-n=4',f'-nthreads={threads}','-gparticle=[{name: geantino, p: 1*GeV}]','-gstreamer=[{format: json, filename: events}]'],cwd=work,env=env,capture_output=True,text=True)
        assert result.returncode == 0, result.stdout+result.stderr
        count = sum(len(json.loads(p.read_text())['events']) for p in work.glob('events*.json') if p.stat().st_size)
        if count != 4: failures.append((mode,threads,'generated events',count))
        result = subprocess.run([binary,str(source/'examples/basic/b1/b1.yaml'),'-sql='+str(database),'-n=10',f'-nthreads={threads}','-seed=12345','-gparticle=[{name: proton, p: 1*GeV, vz: -10*cm}]','-gstreamer=[{format: csv, filename: dose}]'],cwd=work,env=env,capture_output=True,text=True)
        assert result.returncode == 0, result.stdout+result.stderr
        rows=[]
        for p in work.glob('dose*_digitized.csv'):
            if p.stat().st_size: rows.extend(csv.DictReader(p.open(),skipinitialspace=True))
        if not rows or not any(float(row.get('dose',0))>0 for row in rows):
            failures.append((mode,threads,'integrated dose',rows))
        generated=sum(len(list(csv.DictReader(p.open(),skipinitialspace=True))) for p in work.glob('dose*_generated.csv') if p.stat().st_size)
        if generated!=10: failures.append((mode,threads,'mixed generated events',generated))
        if len(rows)>1: failures.append((mode,threads,'duplicate integrated rows',len(rows)))
    if len(sys.argv)>3:
        work=Path(directory)/'normalization';work.mkdir()
        driver=str(Path(sys.argv[3]).resolve())
        result=subprocess.run([driver,'-gstreamer=[{format: csv, filename: normalization}]'],cwd=work,capture_output=True,text=True)
        assert result.returncode==0,result.stdout+result.stderr
        path=work/'normalization_digitized.csv'
        values=list(csv.DictReader(path.open(),skipinitialspace=True))
        assert len(values)==1 and float(values[0]['dose'])==10,(values,'expected 40/4=10 exactly once')
assert not failures, failures
print('Serial, one-worker MT, and two-worker MT event/run publication passed.')
