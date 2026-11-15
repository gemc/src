"""Generated kinematics use internal units for tracked and untracked Lund rows."""
import json
import csv
import re
from pathlib import Path
import subprocess
import sys
import os
import tempfile

for argument in sys.argv[2:]:
    if argument.startswith("--plugin-path="):
        os.environ["GEMC_PLUGIN_PATH"] = argument.split("=", 1)[1]

binary = str(Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory() as directory:
    work = Path(directory)
    (work/'particles.lund').write_text('3 1 1 0 0 0 0 0 0 0\n\n1 -1 0 11 0 0 0 0 1 1.00000013 0.000511 -1 2 3\n2 -1 1 11 0 0 0 0 1 1.00000013 0.000511 -1 2 3\n3 -1 0 11 0 0 0 0 0 0.000511 0.000511 0 -1 2\n')
    formats='{format: json, filename: particles}, {format: csv, filename: mirror}'+(', {format: root, filename: mirror}' if '--root' in sys.argv else '')
    result = subprocess.run([binary, '-n=1', '-nthreads=1', '-gparticlefile=[{format: lund, filename: particles.lund}]', '-gstreamer=['+formats+']'], cwd=work, capture_output=True, text=True)
    assert result.returncode == 0, result.stdout+result.stderr
    particles = json.loads((work/'particles_t0.json').read_text())['events'][0]['generated']['generated']
    assert len(particles) == 3
    for particle in [x for x in particles if x['p']!=0]:
        assert particle['p'] == 1000, particle
        assert [particle[k] for k in ['vx','vy','vz']] == [-10,20,30], particle
        assert particle['theta'] == 0 and particle['phi'] == 0, particle
    zero=[x for x in particles if x['p']==0]
    assert len(zero)==1 and zero[0]['theta']==0 and zero[0]['phi']==0,zero
    assert [zero[0][k] for k in ['vx','vy','vz']]==[0,-10,20],zero
    keys=['p','vx','vy','vz','theta','phi']
    expected=[[row[k] for k in keys] for row in particles]
    rows=list(csv.DictReader((work/'mirror_t0_generated.csv').open(),skipinitialspace=True))
    assert [[float(row[k]) for k in keys] for row in rows]==expected,rows
    if '--root' in sys.argv:
        (work/'inspect.C').write_text(r'''
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include <vector>
#include <iostream>
void inspect() {
 TFile file("mirror_t0.root");auto* tree=dynamic_cast<TTree*>(file.Get("generated"));
 if(!tree){gSystem->Exit(2);return;}
 std::vector<double> *p=nullptr,*vx=nullptr,*vy=nullptr,*vz=nullptr,*theta=nullptr,*phi=nullptr;
 tree->SetBranchAddress("p",&p);tree->SetBranchAddress("vx",&vx);tree->SetBranchAddress("vy",&vy);
 tree->SetBranchAddress("vz",&vz);tree->SetBranchAddress("theta",&theta);tree->SetBranchAddress("phi",&phi);
 tree->GetEntry(0);
 for(size_t i=0;i<p->size();++i)std::cout<<"ROW "<<p->at(i)<<" "<<vx->at(i)<<" "<<vy->at(i)<<" "<<vz->at(i)<<" "<<theta->at(i)<<" "<<phi->at(i)<<std::endl;
 gSystem->Exit(0);
}
''')
        result=subprocess.run(['root','-l','-b','-q','inspect.C'],cwd=work,capture_output=True,text=True)
        assert result.returncode==0,result.stdout+result.stderr
        actual=[[float(x) for x in line.split()[1:]] for line in result.stdout.splitlines() if line.startswith('ROW ')]
        assert actual==expected,(actual,expected)
print('Lund tracked/untracked units, zero momentum, negative vertices and output-bank consistency passed.')
