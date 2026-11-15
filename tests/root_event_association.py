"""Sparse ROOT detector entries join the exact hit-bearing events in parallel JSON output."""
import json
from pathlib import Path
import re
import subprocess
import sys
import os
import tempfile

for argument in sys.argv[2:]:
    if argument.startswith("--plugin-path="):
        os.environ["GEMC_PLUGIN_PATH"] = argument.split("=", 1)[1]

binary=str(Path(sys.argv[1]).resolve());source=Path(__file__).resolve().parents[1]
assert len(sys.argv)>3 and not sys.argv[3].startswith("--"), "ROOT sparse fixture executable is required"
with tempfile.TemporaryDirectory() as directory:
    work=Path(directory)
    database=work/'geometry.db'
    setup=subprocess.run([sys.executable,str(source/'examples/basic/simple_flux/simple_flux.py'),'-f','sqlite','-sql',str(database)],cwd=work,capture_output=True,text=True)
    assert setup.returncode==0,setup.stdout+setup.stderr
    run=subprocess.run([binary,str(source/'examples/basic/simple_flux/simple_flux.yaml'),'-sql='+str(database),'-n=30','-nthreads=1','-seed=12345','-gparticle=[{name: e-, p: 1*GeV, theta: 0*deg, delta_theta: 89*deg}]','-gstreamer=[{format: root, filename: flux}, {format: json, filename: reference}]'],cwd=work,capture_output=True,text=True)
    assert run.returncode==0,run.stdout+run.stderr
    events=json.loads((work/'reference_t0.json').read_text())['events']
    expected=[e['header']['g4local_event'] for e in events if e.get('detectors',{}).get('digitized_by_detector',{}).get('flux')]
    assert 0<len(expected)<30,expected
    (work/'inspect.C').write_text(r'''
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include <vector>
#include <iostream>
void inspect() {
 TFile file("flux_t0.root");
 for (auto name : {"digitized_flux", "true_info_flux"}) {
  auto* tree=dynamic_cast<TTree*>(file.Get(name));
  if (!tree || !tree->GetBranch("g4localEventNumber") || !tree->GetBranch("threadID")) {
   std::cerr<<"Missing detector event keys in "<<name<<std::endl; gSystem->Exit(2); return;
  }
  std::vector<int>* events=nullptr; std::vector<int>* threads=nullptr;
  tree->SetBranchAddress("g4localEventNumber",&events);
  tree->SetBranchAddress("threadID",&threads);
  for(Long64_t i=0;i<tree->GetEntries();++i) {
   tree->GetEntry(i);
   if(events->size()!=1 || threads->size()!=1 || threads->at(0)!=0) {gSystem->Exit(3);return;}
   std::cout<<"KEY "<<name<<" "<<events->at(0)<<std::endl;
  }
 }
}
''')
    result=subprocess.run(['root','-l','-b','-q','inspect.C'],cwd=work,capture_output=True,text=True)
    assert result.returncode==0,result.stdout+result.stderr
    for tree in ['digitized_flux','true_info_flux']:
        actual=[int(x) for x in re.findall(r'KEY '+tree+r' (\d+)',result.stdout)]
        assert actual==expected,(tree,expected,actual)
    if len(sys.argv)>3 and not sys.argv[3].startswith('--'):
        fixture=str(Path(sys.argv[3]).resolve())
        result=subprocess.run([fixture,'-gstreamer=[{format: root, filename: deterministic}]'],cwd=work,capture_output=True,text=True)
        assert result.returncode==0,result.stdout+result.stderr
        (work/'deterministic.C').write_text(r'''
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"
#include <vector>
void deterministic() {
 for(int thread=0;thread<2;++thread) {
  TFile file(Form("deterministic_t%d.root",thread));
  for(auto name : {"true_info_A","digitized_A","true_info_B","digitized_B"}) {
   auto* tree=dynamic_cast<TTree*>(file.Get(name));
   if(!tree){gSystem->Exit(11);return;}
   std::vector<int>* events=nullptr;std::vector<int>* threads=nullptr;
   tree->SetBranchAddress("g4localEventNumber",&events);
   tree->SetBranchAddress("threadID",&threads);
   const bool truthA=std::string(name)=="true_info_A";
   const int expectedCount=truthA?2:1;
   if(tree->GetEntries()!=expectedCount){gSystem->Exit(12);return;}
   for(int i=0;i<expectedCount;++i) {
    tree->GetEntry(i);
    const int expected=truthA?(i==0?17:19):(std::string(name)=="digitized_A"?19:18);
    if(!events||!threads||events->size()!=1||threads->size()!=1||events->at(0)!=expected||threads->at(0)!=thread){gSystem->Exit(13);return;}
   }
  }
 }
 TFile run("deterministic.root");auto* tree=dynamic_cast<TTree*>(run.Get("digitized_dose"));
 if(!tree||tree->GetEntries()!=1||tree->GetBranch("g4localEventNumber")||tree->GetBranch("threadID")){gSystem->Exit(14);return;}
 gSystem->Exit(0);
}
''')
        result=subprocess.run(['root','-l','-b','-q','deterministic.C'],cwd=work,capture_output=True,text=True)
        assert result.returncode==0,result.stdout+result.stderr
        result=subprocess.run([fixture,'-recordZeroEdep','-gstreamer=[{format: root, filename: collision}]'],cwd=work,capture_output=True,text=True)
        assert result.returncode!=0 and 'reserved' in (result.stdout+result.stderr).lower(),result.stdout+result.stderr
print('Sparse ROOT joins, deterministic occupancy, worker IDs, rejected digitization, run schema, and collision checks passed.')
