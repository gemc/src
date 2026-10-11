<!-- TRANSLATIONS:START (managed by bin/translate) -->
**Languages:** [EN](README.md) · [IT](README.it.md) · **日本語** · [FR](README.fr.md) · [中文](README.zh.md)
<!-- TRANSLATIONS:END -->

# GEMC

<table>
  <thead>
    <tr>
      <th width="33%">品質</th>
      <th width="34%">デプロイ</th>
      <th width="33%">ドキュメント</th>
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

GEMC（GEant Monte-Carlo）は、検出器シミュレーションおよび放射線輸送シミュレーションのためのデータベース駆動型の
[Geant4](https://geant4.web.cern.ch) アプリケーションです。検出器の記述を、コンパイル済みの C++ アプリケーションの外部に
保持します。

- ユーザーはセットアップを Python で構築します
- GEMC は実行時にそれらの定義を読み込み、Geant4 シミュレーションのパイプラインを実行します

その目的は、独自の Geant4 アプリケーションを書くことなく検出器システムを試作したいユーザーにとって Geant4 ベースの
シミュレーションを利用しやすくすることであり、同時に高度な検出器応答や出力ワークフローのための C++ 拡張ポイントも
維持することです。

![GEMC のアーキテクチャ](https://gemc.github.io/home/assets/images/gemcArchitecture.svg)

<br/>

## 主な特徴

- [`pygemc`](https://github.com/gemc/pygemc) による Python を中心とした検出器定義
- SQLite または ASCII データベースへのジオメトリおよび材料の保存
- GDML、CAD メッシュからのジオメトリインポート
- ラン番号およびジオメトリのバリエーション
- 組み込みの感応検出器タイプ: `flux`、`gPhotonDetector`、`dosimeter`、`particle_counter`
- カスタムのデジタイズ処理、場、ジェネレータ、出力ストリーマのための C++ プラグイン基盤
- コマンドライン/YAML による粒子定義および LUND ファイルからのイベント生成
- サポートする出力形式: ASCII、CSV、JSON、ROOT
- 磁場サポート（多極子磁場や ASCII マップを含む）
- インタラクティブな Geant4/Qt 可視化およびオフスクリーン画像生成
- PyVista によるジオメトリのプレビュー、VTK エクスポート
- CSV および ROOT 形式に対応した Python アナライザ
- Meson ベースの C++ ビルドと、Linux `amd64` および `arm64` 向けの CI テスト済み Docker イメージ

<br/>

## クイックスタート

GEMC を最も手早く試すには、サンプルを利用してください。

- [Jupyter クイックスタート](https://mybinder.org/v2/gh/gemc/binder-tutorials/main?urlpath=lab/tree/notebooks/basic/quickstart.ipynb)
- [ドキュメントとサンプル](https://gemc.github.io/home/)
- [サンプルギャラリー](https://gemc.github.io/home/examples/)

`counter` 検出器を作成し、そこで 1000 イベントのシミュレーションを実行する最小限のローカルワークフロー:

```shell
gemc-system-template -s counter
cd counter
./counter.py
gemc counter.yaml -n=1000
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
```

テンプレートコマンドが `counter` システムを作成し、
Python スクリプトがジオメトリを構築して SQLite ファイル `gemc.db` に保存します。
`gemc` がシミュレーションを実行し、`gemc-analyzer` が CSV 出力から変数をプロットします。

<br/>

## インストール

インストール手順については[インストールガイド](https://gemc.github.io/home/installation/)を参照してください。


<br/>

## pygemc

検出器システムは [`pygemc`](https://pypi.org/project/pygemc/) を使って構築し、データベースに書き込みます。
flux 型の感応検出器となるボックスを定義する例:

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

ジオメトリスクリプトでは、SQLite または ASCII データベースへの書き込み、PyVista によるジオメトリ表示、VTK.js ファイルへのエクスポートが可能です。

```shell
./counter.py --factory sqlite
./counter.py -pv
./counter.py -pvvtk counter -pvz 0.25
```

<br/>

### PyVista による可視化

PyVista は通常の GEMC ジオメトリワークフローの一部です。Geant4 を実行する前に検出器ジオメトリを確認し、
ドキュメント用に可搬性のある VTK.js シーンをエクスポートし、Python ジオメトリスクリプトの編集中に
配置や回転のミスを発見できます。いくつかの例:
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
        材料
      </a>
    </td>
  </tr>
  <tr>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/scintillator_barrel/scintillator_barrel.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/scintillator_barrel/gemc_view.png"
             alt="Scintillator Barrel PyVista" width="180"><br>
        シンチレータバレル
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
        ブーリアンソリッド
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/cad/cad.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/cad/gemc_view.png"
             alt="CAD Import PyVista" width="180"><br>
        CAD インポート
      </a>
    </td>
  </tr>
</table>

リンクを開くと、GEMC のサンプルから生成されたインタラクティブな PyVista シーンを表示できます。


<br/>

## シミュレーションの実行

GEMC は YAML ファイルと、それに相当するコマンドラインオプションで設定します。簡潔なステアリングファイルで、
ラン番号、イベント数、ジェネレータ、ジオメトリシステム、出力ストリーマ、ワールドボリュームを定義できます。

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

バッチモードで実行:

```shell
gemc counter.yaml
```

Geant4 GUI で実行:

```shell
gemc counter.yaml -gui
```

Geant4 マクロのテキストファイルを実行（次のリリースで利用可能になる予定）:

```shell
gemc counter.yaml -geant4_macro=run.mac
gemc counter.yaml -gui -geant4_macro=vis.mac
```

これに相当する YAML 設定は `geant4_macro: run.mac` です。GEMC は初期化と可視化のセットアップの後、
GUI またはインタラクティブターミナルに入る前にマクロを実行します。相対パスは作業ディレクトリを基準に解決されます。
Geant4 はコメント、エイリアス、ネストした `/control/execute` コマンドを処理します。ファイルが見つからない場合や
コマンドが失敗した場合は、ゼロ以外の終了ステータスを返します。

バッチモードでは、マクロが `/run/beamOn 100` などのコマンドでイベント生成を制御します。GEMC はその後、
`-n` で指定したイベント数や `-run_weights` によるイベント数の割り当てに従って自動的に実行することはありません。
検出器の条件は `-run` を使って準備されます。可視化専用のマクロは、ビーム開始コマンドを含まない限りイベントを生成しません。
`geant4_macro` がない場合、通常のイベント処理は変わりません。

オフスクリーンの Geant4 画像を生成:

```shell
gemc counter.yaml -n=10 \
  -g4view="[{driver: TOOLSSG_OFFSCREEN, segsPerCircle: 200}]" \
  -g4camera="[{phi: -10*deg, theta: 250*deg}]" \
  -g4light="[{phi: 160*deg, theta: 120*deg}]"
```

<br/>

## 出力と解析

出力レイヤーはプラグインベースです。組み込みのストリーマ形式には以下が含まれます。

| 形式    | 備考                                                                                                            |
|---------|------------------------------------------------------------------------------------------------------------------|
| `ascii` | 人間が読めるテキスト出力                                                                                       |
| `csv`   | デジタイズされたヒット、真値情報、生成粒子、追跡された生成粒子のスレッドごとの CSV テーブル |
| `json`  | 構造化されたイベント出力                                                                                         |
| `root`  | ROOT TTree。GEMC が ROOT 対応でビルドされている場合に利用可能                                               |

次のリリースでは、イベント出力にトラックの由来情報を含める機能を有効にできるようになります。

```shell
gemc counter.yaml -save_original_track
gemc counter.yaml -save_all_ancestors
```

`-save_original_track` は、各真値情報のヒットに `otid` を設定します。`-save_all_ancestors` は、
元のトラック ID も有効にし、ヒットを生成した各トラックとその親トラックをたどる完全な連鎖を含む、
重複排除済みの `ancestors` バンクを書き込みます。CSV 出力では別の `_ancestors.csv` ファイルを使用し、
ASCII、JSON、ROOT では、そのバンクをイベント出力に含めます。


`pygemc` は、CSV または ROOT 出力を解析するためのユーティリティを提供します。例えば:

```shell
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
gemc-analyzer out.root E --kind root --detector flux --save energy.png
```


例えば、[`b2` のサンプル](https://gemc.github.io/home/examples/basic/b2/)を
`gemc b2.yaml -n=1000` で実行した後、アナライザは以下を生成します。

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

## ドキュメント

- [GEMC ホームページ](https://gemc.github.io/home/)
- [スレッドスケーリング](https://gemc.github.io/home/documentation/support/thread_scaling) — マルチスレッドのベンチマーク
  およびスレッド数を変えたスケーリング測定
- [プロファイリング](https://gemc.github.io/home/documentation/support/valgrind_profile) — Valgrind callgrind のメトリクス
  とその読み方
- [Python API リポジトリ](https://github.com/gemc/pygemc)
- [CLAS12 GEMC システムリポジトリ](https://github.com/gemc/clas12-systems)


<br/>

## コントリビューション

通常のプルリクエストによる貢献を歓迎します。プルリクエストを作成する前に、関連する Meson テストを実行し、
変更範囲を絞ってください。以下を参照してください。

- [`CONTRIBUTING.md`](CONTRIBUTING.md)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md)
- [`SECURITY.md`](SECURITY.md)
- [CI ワークフローガイド](.github/workflows/README.md)

<br/>

## 引用

科学研究で GEMC を使用する場合は、以下を引用してください。

> M. Ungaro and the CLAS Collaboration, "The CLAS12 Geant4 simulation," *Nuclear Instruments and Methods
> in Physics Research Section A* **959**, 163422 (2020). https://doi.org/10.1016/j.nima.2020.163422

BibTeX およびその他の引用形式は [`CITATION.md`](CITATION.md) にあります。

<br/>

## ライセンス

GEMC は [Apache ライセンス、バージョン 2.0](LICENSE) の下でライセンスされています。帰属表示および
サードパーティへの謝辞については [`NOTICE`](NOTICE) を参照してください。本ソフトウェアは、Geant4、CLHEP、Qt、ROOT、
SQLite、Assimp など、別途ライセンスが適用されるサードパーティコンポーネントにも依存しています。

<br/>

## ロードマップ

現在の開発計画については[プロジェクトロードマップ](https://github.com/orgs/gemc/projects/1/views/4)を参照してください。
