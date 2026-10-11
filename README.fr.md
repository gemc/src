<!-- TRANSLATIONS:START (managed by bin/translate) -->
**Languages:** [EN](README.md) · [IT](README.it.md) · [日本語](README.ja.md) · **FR** · [中文](README.zh.md)
<!-- TRANSLATIONS:END -->

# GEMC

<table>
  <thead>
    <tr>
      <th width="33%">Qualité</th>
      <th width="34%">Déploiement</th>
      <th width="33%">Documentation</th>
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

GEMC, le GEant Monte-Carlo, est une application [Geant4](https://geant4.web.cern.ch) pilotée par une base de données,
destinée aux simulations de détecteurs et de transport du rayonnement.
Elle conserve les descriptions des détecteurs en dehors de l'application C++ compilée :

- les utilisateurs construisent leurs configurations en Python
- GEMC charge ces définitions à l'exécution et exécute la chaîne de simulation Geant4

L'objectif est de rendre la simulation basée sur Geant4 accessible aux utilisateurs qui souhaitent
prototyper des systèmes de détecteurs sans écrire une application Geant4 sur mesure,
tout en conservant des points d'extension C++ pour la réponse avancée des détecteurs et les traitements de sortie.

![Architecture de GEMC](https://gemc.github.io/home/assets/images/gemcArchitecture.svg)

<br/>

## Points forts

- Définition des détecteurs principalement en Python grâce à [`pygemc`](https://github.com/gemc/pygemc)
- Stockage de la géométrie et des matériaux dans des bases de données SQLite ou ASCII
- Import de géométries depuis GDML et des maillages CAO
- Gestion des numéros de run et des variantes de géométrie
- Types de détecteurs sensibles intégrés : `flux`, `gPhotonDetector`, `dosimeter`, `particle_counter`
- Infrastructure de plugins C++ pour la numérisation, les champs, les générateurs et les modules de sortie personnalisés
- Génération d'événements à partir de définitions de particules en ligne de commande/YAML et de fichiers LUND
- Formats de sortie pris en charge : ASCII, CSV, JSON, ROOT
- Prise en charge des champs magnétiques, notamment des champs multipolaires et des cartes ASCII
- Visualisation interactive Geant4/Qt et génération d'images hors écran
- Aperçu de la géométrie avec PyVista, export VTK
- Analyseur Python pour les formats CSV et ROOT
- Compilation C++ avec meson et images Docker testées en intégration continue pour Linux `amd64` et `arm64`

<br/>

## Démarrage rapide

Le moyen le plus rapide d'essayer GEMC passe par les exemples :

- [Démarrage rapide Jupyter](https://mybinder.org/v2/gh/gemc/binder-tutorials/main?urlpath=lab/tree/notebooks/basic/quickstart.ipynb)
- [Documentation et exemples](https://gemc.github.io/home/)
- [Galerie d'exemples](https://gemc.github.io/home/examples/)

Un flux de travail local minimal qui crée un détecteur `counter` et y simule le passage de 1000 événements :

```shell
gemc-system-template -s counter
cd counter
./counter.py
gemc counter.yaml -n=1000
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
```

La commande de création à partir d'un modèle crée un système `counter`,
le script Python construit la géométrie et la stocke dans un fichier SQLite `gemc.db`,
`gemc` exécute la simulation et `gemc-analyzer` trace une variable à partir de la sortie CSV.

<br/>

## Installation

Pour les instructions d'installation, consultez le [guide d'installation](https://gemc.github.io/home/installation/)


<br/>

## pygemc

Les systèmes de détecteurs sont construits et enregistrés dans des bases de données à l'aide de
[`pygemc`](https://pypi.org/project/pygemc/).
Un exemple de définition d'une boîte sensible de type flux :

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

Les scripts de géométrie peuvent écrire des bases de données SQLite ou ASCII, afficher la géométrie avec PyVista
ou exporter des fichiers VTK.js :

```shell
./counter.py --factory sqlite
./counter.py -pv
./counter.py -pvvtk counter -pvz 0.25
```

<br/>

### Visualisation PyVista

PyVista fait partie du flux de travail habituel de GEMC pour la géométrie. Il permet d'inspecter la géométrie des
détecteurs avant d'exécuter Geant4, d'exporter des scènes VTK.js portables pour la documentation et de détecter
les erreurs de placement ou de rotation lors de la modification des scripts de géométrie Python. Quelques exemples :
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
        Matériaux
      </a>
    </td>
  </tr>
  <tr>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/scintillator_barrel/scintillator_barrel.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/scintillator_barrel/gemc_view.png"
             alt="Scintillator Barrel PyVista" width="180"><br>
        Tonneau de scintillateurs
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
        Solides booléens
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/cad/cad.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/cad/gemc_view.png"
             alt="CAD Import PyVista" width="180"><br>
        Import CAO
      </a>
    </td>
  </tr>
</table>

Ouvrez les liens vers les scènes PyVista interactives générées à partir des exemples GEMC.


<br/>

## Exécution des simulations

GEMC se configure à l'aide de fichiers YAML et d'options équivalentes en ligne de commande. Un fichier de pilotage
compact peut définir le numéro de run, le nombre d'événements, le générateur, les systèmes de géométrie, les modules
de sortie et le volume monde :

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

Exécution en mode batch :

```shell
gemc counter.yaml
```

Exécution avec l'interface graphique Geant4 :

```shell
gemc counter.yaml -gui
```

Exécuter un fichier texte contenant une macro Geant4 (à venir dans la prochaine version) :

```shell
gemc counter.yaml -geant4_macro=run.mac
gemc counter.yaml -gui -geant4_macro=vis.mac
```

Le paramètre YAML équivalent est `geant4_macro: run.mac`. GEMC exécute la macro après l'initialisation et la
configuration de la visualisation, avant d'entrer dans l'interface graphique ou le terminal interactif. Les chemins
relatifs sont résolus à partir du répertoire de travail ; Geant4 gère les commentaires, les alias et les commandes
`/control/execute` imbriquées. Les fichiers manquants et les commandes en échec entraînent un code de sortie non nul.

En mode batch, la macro pilote la génération d'événements avec des commandes telles que `/run/beamOn 100` ; GEMC
n'exécute pas ensuite automatiquement les événements définis par `-n` ou leur répartition selon `-run_weights`.
Les conditions du détecteur sont préparées avec `-run`. Une macro dédiée à la visualisation ne génère aucun
événement sauf si elle contient une commande de lancement du faisceau. Sans `geant4_macro`, le traitement
habituel des événements reste inchangé.

Générer une image Geant4 hors écran :

```shell
gemc counter.yaml -n=10 \
  -g4view="[{driver: TOOLSSG_OFFSCREEN, segsPerCircle: 200}]" \
  -g4camera="[{phi: -10*deg, theta: 250*deg}]" \
  -g4light="[{phi: 160*deg, theta: 120*deg}]"
```

<br/>

## Sortie et analyse

La couche de sortie repose sur des plugins. Les formats des modules de sortie intégrés comprennent :

| Format  | Remarques                                                                                                        |
|---------|------------------------------------------------------------------------------------------------------------------|
| `ascii` | Sortie texte lisible par un humain                                                                                |
| `csv`   | Tables CSV par thread pour les impacts numérisés, les informations de vérité, les particules générées et les particules générées suivies |
| `json`  | Sortie structurée des événements                                                                                  |
| `root`  | TTrees ROOT, disponibles lorsque GEMC est compilé avec ROOT                                                        |

Dans la prochaine version, il sera possible d'activer l'enregistrement de la provenance des trajectoires en sortie :

```shell
gemc counter.yaml -save_original_track
gemc counter.yaml -save_all_ancestors
```

`-save_original_track` renseigne `otid` dans chaque impact contenant les informations de vérité.
`-save_all_ancestors` active également les identifiants des trajectoires d'origine et écrit une banque `ancestors`
sans doublons contenant chaque trajectoire à l'origine d'un impact et la chaîne complète de ses ancêtres.
La sortie CSV utilise un fichier `_ancestors.csv` distinct ; ASCII, JSON et ROOT incluent cette banque
dans leur sortie d'événements.


`pygemc` fournit quelques utilitaires pour analyser les sorties CSV ou ROOT. Par exemple :

```shell
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
gemc-analyzer out.root E --kind root --detector flux --save energy.png
```


Par exemple, après avoir exécuté l'[exemple `b2`](https://gemc.github.io/home/examples/basic/b2/) avec
`gemc b2.yaml -n=1000`, l'analyseur produit :

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

## Documentation

- [Page d'accueil de GEMC](https://gemc.github.io/home/)
- [Passage à l'échelle multithread](https://gemc.github.io/home/documentation/support/thread_scaling) — tests de
  performances multithreads et mesures pour différents nombres de threads
- [Profilage](https://gemc.github.io/home/documentation/support/valgrind_profile) — métriques callgrind de Valgrind
  et leur interprétation
- [Dépôt de l'API Python](https://github.com/gemc/pygemc)
- [Dépôt des systèmes GEMC pour CLAS12](https://github.com/gemc/clas12-systems)


<br/>

## Contribuer

Les contributions sont les bienvenues sous forme de pull requests. Avant d'ouvrir une pull request, exécutez les
tests meson pertinents et limitez les modifications à l'objectif visé. Voir :

- [`CONTRIBUTING.md`](CONTRIBUTING.md)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md)
- [`SECURITY.md`](SECURITY.md)
- [Guide des workflows CI](.github/workflows/README.md)

<br/>

## Citation

Si vous utilisez GEMC dans un travail scientifique, citez :

> M. Ungaro and the CLAS Collaboration, "The CLAS12 Geant4 simulation," *Nuclear Instruments and Methods
> in Physics Research Section A* **959**, 163422 (2020). https://doi.org/10.1016/j.nima.2020.163422

La référence au format BibTeX et dans d'autres formats de citation est disponible dans [`CITATION.md`](CITATION.md).

<br/>

## Licence

GEMC est distribué sous la [licence Apache, version 2.0](LICENSE) ; voir [`NOTICE`](NOTICE) pour les mentions
d'attribution et les remerciements aux tiers. Le logiciel dépend également de composants tiers sous licences
distinctes, notamment Geant4, CLHEP, Qt, ROOT, SQLite et Assimp.

<br/>

## Feuille de route

Consultez la [feuille de route du projet](https://github.com/orgs/gemc/projects/1/views/4) pour les plans de
développement actuels.
