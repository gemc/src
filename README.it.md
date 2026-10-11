<!-- TRANSLATIONS:START (managed by bin/translate) -->
**Languages:** [EN](README.md) · **IT** · [日本語](README.ja.md) · [FR](README.fr.md) · [中文](README.zh.md)
<!-- TRANSLATIONS:END -->

# GEMC

<table>
  <thead>
    <tr>
      <th width="33%">Qualità</th>
      <th width="34%">Distribuzione</th>
      <th width="33%">Documentazione</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td align="center" valign="top">
        <a href="https://github.com/gemc/src/actions/workflows/test.yml">
          <img alt="Test" src="https://github.com/gemc/src/actions/workflows/test.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/sanitize.yml">
          <img alt="Sanitize" src="https://github.com/gemc/src/actions/workflows/sanitize.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/codeql.yml">
          <img alt="CodeQL" src="https://github.com/gemc/src/actions/workflows/codeql.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/thread_scaling.yml">
          <img alt="Thread Scaling"
               src="https://github.com/gemc/src/actions/workflows/thread_scaling.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/valgrind_profile.yml">
          <img alt="Valgrind Profile"
               src="https://github.com/gemc/src/actions/workflows/valgrind_profile.yml/badge.svg">
        </a>
      </td>
      <td align="center" valign="top">
        <a href="https://github.com/gemc/src/actions/workflows/deploy.yml">
          <img alt="Deploy" src="https://github.com/gemc/src/actions/workflows/deploy.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/binary_tarballs.yml">
          <img alt="Binary Tarballs"
               src="https://github.com/gemc/src/actions/workflows/binary_tarballs.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/release_tarballs.yml">
          <img alt="Release Tarballs"
               src="https://github.com/gemc/src/actions/workflows/release_tarballs.yml/badge.svg?event=release">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/macos_tarball.yml">
          <img alt="macOS Tarball"
               src="https://github.com/gemc/src/actions/workflows/macos_tarball.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/dev_release.yml">
          <img alt="Nightly Dev Release"
               src="https://github.com/gemc/src/actions/workflows/dev_release.yml/badge.svg">
        </a><br/>
        <a href="https://pypi.org/project/pygemc/">
          <img alt="pygemc PyPI" src="https://img.shields.io/pypi/v/pygemc.svg?cacheSeconds=300">
        </a><br/>
        <a href="https://hub.docker.com/r/gemc/gemc">
          <img alt="Docker Pulls" src="https://img.shields.io/docker/pulls/gemc/gemc.svg">
        </a>
      </td>
      <td align="center" valign="top">
        <a href="https://gemc.github.io/home/">
          <img alt="GEMC documentation" src="https://img.shields.io/badge/docs-gemc.github.io-blue.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/doxygen.yml">
          <img alt="Doxygen" src="https://github.com/gemc/src/actions/workflows/doxygen.yml/badge.svg">
        </a>
      </td>
    </tr>
  </tbody>
</table>

GEMC, il GEant Monte-Carlo, è un'applicazione [Geant4](https://geant4.web.cern.ch) basata su database per
simulazioni di rivelatori e di trasporto della radiazione. Mantiene le descrizioni dei rivelatori al di fuori
dell'applicazione C++ compilata:

- gli utenti costruiscono le configurazioni in Python
- GEMC carica tali definizioni a runtime ed esegue la pipeline di simulazione Geant4

L'obiettivo è rendere la simulazione basata su Geant4 accessibile agli utenti che vogliono
prototipare sistemi di rivelatori senza scrivere un'applicazione Geant4 personalizzata,
preservando al contempo i punti di estensione in C++ per workflow avanzati di risposta del rivelatore e di output.

![Architettura di GEMC](https://gemc.github.io/home/assets/images/gemcArchitecture.svg)

<br/>

## Punti salienti

- Definizione dei rivelatori incentrata su Python tramite [`pygemc`](https://github.com/gemc/pygemc)
- Archiviazione di geometrie e materiali in database SQLite o ASCII
- Importazione di geometrie da GDML e mesh CAD
- Varianti in base al numero di run e alla geometria
- Tipi di rivelatori sensibili integrati: `flux`, `gPhotonDetector`, `dosimeter`, `particle_counter`
- Infrastruttura di plugin C++ per digitizzazione, campi, generatori e streamer di output personalizzati
- Generazione di eventi da definizioni di particelle da riga di comando/YAML e da file LUND
- Formati di output supportati: ASCII, CSV, JSON, ROOT
- Supporto per campi magnetici, inclusi campi multipolari e mappe ASCII
- Visualizzazione interattiva Geant4/Qt e generazione di immagini off-screen
- Anteprima della geometria con PyVista, esportazione VTK
- Analizzatore Python per formati CSV e ROOT
- Compilazione C++ basata su meson con immagini Docker testate in CI per Linux `amd64` e `arm64`

<br/>

## Avvio rapido

Il modo più veloce per provare GEMC è attraverso gli esempi:

- [Avvio rapido con Jupyter](https://mybinder.org/v2/gh/gemc/binder-tutorials/main?urlpath=lab/tree/notebooks/basic/quickstart.ipynb)
- [Documentazione ed esempi](https://gemc.github.io/home/)
- [Galleria di esempi](https://gemc.github.io/home/examples/)

Un workflow locale minimo che crea un rivelatore `counter` e simula 1000 eventi al suo interno:

```shell
gemc-system-template -s counter
cd counter
./counter.py
gemc counter.yaml -n=1000
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
```

Il comando di creazione del modello genera un sistema `counter`,
lo script Python costruisce la geometria e la memorizza in un file SQLite `gemc.db`,
`gemc` esegue la simulazione e `gemc-analyzer` genera il grafico di una variabile dall'output CSV.

<br/>

## Installazione

Per le istruzioni di installazione consulta la [guida all'installazione](https://gemc.github.io/home/installation/)


<br/>

## pygemc

I sistemi di rivelatori vengono costruiti e scritti nei database tramite [`pygemc`](https://pypi.org/project/pygemc/).
Un esempio di definizione di un volume sensibile a forma di parallelepipedo per misurare il flusso:

```python
from pygemc import GVolume, autogeometry

cfg = autogeometry("examples", "counter")

flux = GVolume("flux_box")
flux.description = "air flux box"
flux.make_box(40.0, 40.0, 2.0)
flux.set_position(0, 0, 100)
flux.material = "G4_AIR"
flux.color = "3399FF"
flux.digitization = "flux"
flux.set_identifier("box", 2)
flux.publish(cfg)
```

Gli script di geometria possono scrivere database SQLite o ASCII, mostrare la geometria con PyVista o esportare
file VTK.js:

```shell
./counter.py --factory sqlite
./counter.py -pv
./counter.py -pvvtk counter -pvz 0.25
```

<br/>

### Visualizzazione PyVista

PyVista fa parte del normale workflow di geometria di GEMC. Permette agli utenti di ispezionare la geometria del
rivelatore prima di eseguire Geant4, di esportare scene VTK.js portabili per la documentazione e di individuare
errori di posizionamento o rotazione durante la modifica degli script Python di geometria. Alcuni esempi:
<table>
  <tr>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/b2/b2.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/b2/gemc_view.png"
             alt="B2 PyVista" width="180"><br>
        Geant4 basic/b2
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/materials/material.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/materials/gemc_view.png"
             alt="Materials PyVista" width="180"><br>
        Materiali
      </a>
    </td>
  </tr>
  <tr>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/scintillator_barrel/scintillator_barrel.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/scintillator_barrel/gemc_view.png"
             alt="Scintillator Barrel PyVista" width="180"><br>
        Rivelatore cilindrico a scintillatori
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/dc/dc.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/dc/gemc_view.png"
             alt="Materials PyVista" width="180"><br>
        CLAS12 DC
      </a>
    </td>
  </tr>
  <tr>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/boolean_solids/boolean_solids.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/boolean_solids/gemc_view.png"
             alt="Boolean Solids PyVista" width="180"><br>
        Solidi booleani
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/cad/cad.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/cad/gemc_view.png"
             alt="CAD Import PyVista" width="180"><br>
        Importazione CAD
      </a>
    </td>
  </tr>
</table>

Apri le scene PyVista interattive collegate, generate dagli esempi di GEMC.


<br/>

## Esecuzione delle simulazioni

GEMC si configura con file YAML e con opzioni da riga di comando equivalenti. Un file di configurazione compatto
può definire il numero di run, il numero di eventi, il generatore, i sistemi di geometria, gli streamer di output
e il volume del mondo:

```yaml
runno: 1
n: 100
nthreads: 1

gparticle:
  - name: proton
    p: 1500
    vz: -5.0

gsystem:
  - name: counter
    factory: sqlite

gstreamer:
  - filename: counter
    format: csv
  - filename: counter
    format: root

root: G4Box, 15*cm, 15*cm, 15*cm, G4_AIR
```

Esecuzione in modalità batch:

```shell
gemc counter.yaml
```

Esecuzione con la GUI di Geant4:

```shell
gemc counter.yaml -gui
```

Esecuzione di una macro Geant4 contenuta in un file di testo (in arrivo nella prossima release):

```shell
gemc counter.yaml -geant4_macro=run.mac
gemc counter.yaml -gui -geant4_macro=vis.mac
```

L'impostazione YAML equivalente è `geant4_macro: run.mac`. GEMC esegue la macro dopo l'inizializzazione e la
configurazione della visualizzazione, prima di entrare nella GUI o nel terminale interattivo. I percorsi relativi
sono risolti rispetto alla directory di lavoro; Geant4 gestisce commenti, alias e comandi `/control/execute`
annidati. File mancanti e comandi falliti producono un codice di uscita diverso da zero.

In modalità batch, la macro controlla la generazione degli eventi con comandi come `/run/beamOn 100`; GEMC non
esegue poi automaticamente gli eventi specificati da `-n` o ripartiti tramite `-run_weights`. Le condizioni del
rivelatore vengono preparate con `-run`. Una macro di sola visualizzazione non genera eventi a meno che non
contenga un comando di avvio del fascio. Senza `geant4_macro`, l'elaborazione abituale degli eventi resta invariata.

Generazione di un'immagine off-screen di Geant4:

```shell
gemc counter.yaml -n=10 \
  -g4view="[{driver: TOOLSSG_OFFSCREEN, segsPerCircle: 200}]" \
  -g4camera="[{phi: -10*deg, theta: 250*deg}]" \
  -g4light="[{phi: 160*deg, theta: 120*deg}]"
```

<br/>

## Output e analisi

Il livello di output è basato su plugin. I formati degli streamer integrati includono:

| Formato | Note |
|---------|------------------------------------------------------------------------------------------------------------------|
| `ascii` | Output testuale leggibile dall'uomo |
| `csv`   | Tabelle CSV per thread per hit digitizzati, informazioni di verità, particelle generate e particelle generate tracciate |
| `json`  | Output strutturato degli eventi |
| `root`  | TTree di ROOT, disponibili quando GEMC è compilato con ROOT |

Nella prossima release sarà possibile abilitare la provenienza delle tracce nell'output degli eventi:

```shell
gemc counter.yaml -save_original_track
gemc counter.yaml -save_all_ancestors
```

`-save_original_track` popola `otid` in ciascun hit contenente informazioni di verità. `-save_all_ancestors` abilita
anche gli ID delle tracce originali e scrive un banco `ancestors` privo di duplicati contenente ogni traccia che
produce hit e la sua catena completa di tracce progenitrici. L'output CSV usa un file `_ancestors.csv` separato;
ASCII, JSON e ROOT includono il banco nell'output degli eventi.


`pygemc` fornisce alcune utility per analizzare l'output CSV o ROOT. Ad esempio:

```shell
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
gemc-analyzer out.root E --kind root --detector flux --save energy.png
```


Ad esempio, dopo aver eseguito l'[esempio `b2`](https://gemc.github.io/home/examples/basic/b2/) con
`gemc b2.yaml -n=1000`, l'analizzatore produce:

<table>
  <tr>
    <td align="center">
      <img src="https://gemc.github.io/home/assets/images/examples/b2/analyzer_totEdep.png" width="100%"/>
      <br/>
      <sub><code>gemc-analyzer b2_t0_digitized.csv totEdep --kind csv</code></sub>
    </td>
    <td align="center">
      <img src="https://gemc.github.io/home/assets/images/examples/b2/analyzer_true_energy.png" width="100%"/>
      <br/>
      <sub><code>gemc-analyzer b2_t0_true_info.csv E --kind csv --data true_info</code></sub>
    </td>
  </tr>
</table>

<br/>

## Documentazione

- [Pagina principale di GEMC](https://gemc.github.io/home/)
- [Scalabilità con il numero di thread](https://gemc.github.io/home/documentation/support/thread_scaling) —
  benchmark multithread e test di scalabilità al variare del numero di thread
- [Profilazione](https://gemc.github.io/home/documentation/support/valgrind_profile) — metriche callgrind di
  Valgrind e come leggerle
- [Repository dell'API Python](https://github.com/gemc/pygemc)
- [Repository dei sistemi GEMC per CLAS12](https://github.com/gemc/clas12-systems)


<br/>

## Come contribuire

I contributi sono benvenuti attraverso le normali pull request. Prima di aprire una pull request, esegui i test
meson pertinenti e mantieni le modifiche circoscritte. Vedi:

- [`CONTRIBUTING.md`](CONTRIBUTING.md)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md)
- [`SECURITY.md`](SECURITY.md)
- [Guida ai workflow CI](.github/workflows/README.md)

<br/>

## Citazione

Se usi GEMC in un lavoro scientifico, cita:

> M. Ungaro e la collaborazione CLAS, "The CLAS12 Geant4 simulation," *Nuclear Instruments and Methods
> in Physics Research Section A* **959**, 163422 (2020). https://doi.org/10.1016/j.nima.2020.163422

BibTeX e altri formati di citazione sono in [`CITATION.md`](CITATION.md).

<br/>

## Licenza

GEMC è distribuito con la [licenza Apache, versione 2.0](LICENSE); vedi [`NOTICE`](NOTICE) per le attribuzioni e i
riconoscimenti relativi a terze parti. Il software dipende inoltre da componenti di terze parti con licenze
separate, tra cui Geant4, CLHEP, Qt, ROOT, SQLite e Assimp.

<br/>

## Roadmap

Consulta la [roadmap del progetto](https://github.com/orgs/gemc/projects/1/views/4) per i piani di sviluppo attuali.
