"""Save/load preserves unset options, values, collections, and false switches."""
from pathlib import Path
import json
import subprocess
import sys
import os
import tempfile

for argument in sys.argv[2:]:
    if argument.startswith("--plugin-path="):
        os.environ["GEMC_PLUGIN_PATH"] = argument.split("=", 1)[1]
import yaml

binary = str(Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    for name in ['initial','replay']:
        (root/name).mkdir()
    result = subprocess.run([binary, '-n=1', '-nthreads=1', '-seed=12345', '-print_summary=false', '-gparticle=[{name: geantino, p: 1*GeV}]', '-recordZeroEdep', '-gparticlefile=[]', '-gstreamer=[{format: json, filename: particles}]'], cwd=root/'initial', capture_output=True, text=True)
    assert result.returncode == 0, result.stdout+result.stderr
    saved = root/'initial/gemc.saved_configuration.yaml'
    config = yaml.safe_load(saved.read_text())
    assert config['gui'] is False and config['print_summary'] is False, config
    for key in ['run_weights','geant4_macro','gmultipoles','gfields','gsystem']:
        assert config[key] is None, (key,config[key])
    assert config['seed'] == 12345 or config['seed'] == '12345'
    assert config['gparticlefile'] == []
    assert config['recordZeroEdep'] is True
    assert config['gstreamer'][0]['type'] is None
    result = subprocess.run([binary, str(saved)], cwd=root/'replay', capture_output=True, text=True)
    assert result.returncode == 0, result.stdout+result.stderr
    replay = yaml.safe_load((root/'replay/gemc.saved_configuration.yaml').read_text())
    for key in ['gui','print_summary','seed','gparticle','gstreamer','run_weights','gsystem']:
        assert replay[key] == config[key], (key,config[key],replay[key])
    original=json.loads((root/'initial/particles_t0.json').read_text())['events'][0]['generated']
    repeated=json.loads((root/'replay/particles_t0.json').read_text())['events'][0]['generated']
    assert repeated==original,(original,repeated)
print('Configuration typed-null/boolean/empty/nested values and CLI replay passed.')
