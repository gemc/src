"""Bare strengths use the documented Tesla default; explicit units retain meaning."""
import math
from pathlib import Path
import re
import subprocess
import sys
import os
import tempfile

for argument in sys.argv[2:]:
    if argument.startswith("--plugin-path="):
        os.environ["GEMC_PLUGIN_PATH"] = argument.split("=", 1)[1]

binary = str(Path(sys.argv[1]).resolve())
units = {'T': 1, 'mT': 1e-3, 'G': 1e-4, 'kG': 0.1, 'uT': 1e-6, 'nT': 1e-9}
with tempfile.TemporaryDirectory() as directory:
    for expression, expected in [('1', 1), ('1*tesla', 1), ('10000*gauss', 1), ('-1', -1), ('0', 0), ('0.5*tesla', 0.5)]:
        result = subprocess.run([binary, '-gmultipoles=[{name: field, pole_number: 2, rotaxis: Z, strength: '+expression+'}]', '-fieldAt=0*mm 0*mm 0*mm'], cwd=directory, capture_output=True, text=True, check=True)
        match = re.search(r'Bx=([-+\deE.]+) (\w+)', result.stdout)
        assert match, result.stdout
        actual = float(match[1])*units[match[2]]
        assert math.isclose(actual, expected, rel_tol=1e-6, abs_tol=1e-12), (expression, expected, actual)
    result = subprocess.run([binary, '-gmultipoles=[{name: centered, pole_number: 4, rotaxis: Z, strength: 1*tesla, vx: 10*cm}]', '-fieldAt=100*mm 0*mm 0*mm'], cwd=directory, capture_output=True, text=True, check=True)
    assert 'Bx=0 G' in result.stdout and 'By=0 G' in result.stdout, result.stdout
    rotated=subprocess.run([binary,'-gmultipoles=[{name: rotated, pole_number: 2, rotaxis: Z, strength: 1*tesla, rotation_angle: 90*deg}]','-fieldAt=0*mm 0*mm 0*mm'],cwd=directory,capture_output=True,text=True,check=True)
    match=re.search(r'By=([-+\deE.]+) (\w+)',rotated.stdout)
    assert match and math.isclose(float(match[1])*units[match[2]],1,rel_tol=1e-6),rotated.stdout
print('Bare/explicit field strengths and existing origin units passed.')
