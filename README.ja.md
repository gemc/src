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
        </a><br/>
        <a href="https://github.com/gemc/pygemc/actions/workflows/pygemc_tests.yml">
          <img alt="pygemc tests"
               src="https://github.com/gemc/pygemc/actions/workflows/pygemc_tests.yml/badge.svg">
        </a><br/>
        <a href="https://hub.docker.com/r/gemc/gemc">
          <img alt="Docker Pulls" src="https://img.shields.io/docker/pulls/gemc/gemc.svg">
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
          <img alt="Nightly gemc Release"
               src="https://github.com/gemc/src/actions/workflows/dev_release.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/pygemc/actions/workflows/publish_pypi.yml">
          <img alt="Publish PyPI"
               src="https://github.com/gemc/pygemc/actions/workflows/publish_pypi.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/pygemc/actions/workflows/dev_release.yml">
          <img alt="Nightly pygemc Release"
               src="https://github.com/gemc/pygemc/actions/workflows/dev_release.yml/badge.svg">
        </a><br/>
        <a href="https://pypi.org/project/pygemc/">
          <img alt="pygemc PyPI" src="https://img.shields.io/pypi/v/pygemc.svg?cacheSeconds=300">
        </a>
      </td>
      <td align="center" valign="top">
        <a href="https://gemc.github.io/home/">
          <img alt="GEMC documentation" src="https://img.shields.io/badge/docs-gemc.github.io-blue.svg">
        </a><br/>
        <a href="https://github.com/gemc/src/actions/workflows/doxygen.yml">
          <img alt="Doxygen" src="https://github.com/gemc/src/actions/workflows/doxygen.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/home/actions/workflows/jekyll.yml">
          <img alt="Homepage" src="https://github.com/gemc/home/actions/workflows/jekyll.yml/badge.svg">
        </a><br/>
        <a href="https://github.com/gemc/binder-tutorials/actions/workflows/publish-image.yml">
          <img alt="Publish Jupyter Image"
               src="https://github.com/gemc/binder-tutorials/actions/workflows/publish-image.yml/badge.svg">
        </a>
      </td>
    </tr>
  </tbody>
</table>

**GEMC** は、検出器および放射線輸送シミュレーションのためのデータベース駆動型の
[Geant4](https://geant4.web.cern.ch) アプリケーションです。Python API により、Geant4 のコードを書かずに
複雑な検出器構成を定義できます。

検出器の記述は実行ファイルから分離されています。Python で記述を作成し、ASCII または SQLite のデータベースに
保存すると、GEMC が実行時にそれらを読み込んで Geant4 シミュレーションを実行します。

C++ プラグインは、カスタムの検出器応答、磁場、イベントジェネレーター、出力形式をサポートします。

![GEMC のアーキテクチャ](https://gemc.github.io/home/assets/images/gemcArchitecture.svg)

<br/>

## 主な特徴

- Python を中心とした検出器定義
- [`pyvista`](https://pyvista.org) によるジオメトリのプレビューと VTK エクスポート
- ASCII、SQLite データベース
- GDML および CAD ファイルからのジオメトリのインポート
- インタラクティブな Geant4/Qt 可視化とオフスクリーン画像生成
- 組み込みの有感検出器タイプ: `flux`、`gPhotonDetector`、`dosimeter`、`particle_counter`
- ASCII、CSV、JSON、[ROOT](https://root.cern) 形式でのシミュレーション出力
- GUI、コマンドライン、YAML および LUND ファイルからのイベント生成
- 多重極磁場や ASCII マップを含む磁場サポート
- シミュレーション出力（CSV、ROOT）の読み込みとプロットを行う Python `analyzer` モジュール
- ジオメトリのバリエーションおよびラン番号に応じた設定
- Linux `amd64` および `arm64` 向けに CI でテスト済みの Docker イメージを備えた meson ベースの C++ ビルド


<br/>

## クイックスタート

GEMC を試す最も手早い方法は、サンプルを使うことです:

- [Jupyter クイックスタート](https://mybinder.org/v2/gh/gemc/binder-tutorials/main?urlpath=lab/tree/notebooks/basic/quickstart.ipynb)
- [ドキュメントとサンプル](https://gemc.github.io/home/)
- [サンプルギャラリー](https://gemc.github.io/home/examples/)

`counter` 検出器を作成し、その検出器で 1000 イベントを実行する最小限のローカルワークフローです:

```shell
gemc-system-template -s counter
cd counter
./counter.py
gemc counter.yaml -n=1000
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
```

テンプレートコマンドは `counter` システムを作成し、
Python スクリプトがジオメトリを構築して SQLite ファイル `gemc.db` に保存します。
`gemc` がシミュレーションを実行し、`gemc-analyzer` が CSV 出力から変数をプロットします。

<br/>

## インストール

インストール手順については、[インストールガイド](https://gemc.github.io/home/installation/)を参照してください。

Python 環境では Python 3.10〜3.14 を使用し、`pymeshlab` を含みます。次回リリースで導入予定:
pygemc サブプロジェクトが、インストール済みのサポート対象 Python を選択し、両方の環境を管理します。
これには `meson install` 時の依存関係のインストールも含まれます。


<br/>

## pygemc

検出器システムは [`pygemc`](https://pypi.org/project/pygemc/) を使って構築し、データベースに書き込みます。
有感検出器として機能する flux ボックスを定義する例:

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

ジオメトリスクリプトでは、SQLite または ASCII データベースへの書き込み、PyVista によるジオメトリ表示、
VTK.js ファイルのエクスポートが可能です:

```shell
./counter.py --factory sqlite
./counter.py -pv
./counter.py -pvvtk counter -pvz 0.25
```

<br/>

### PyVista による可視化

PyVista は通常の GEMC ジオメトリワークフローの一部です。Geant4 の実行前に検出器ジオメトリを確認したり、
ドキュメント用に可搬性のある VTK.js シーンをエクスポートしたり、Python ジオメトリスクリプトの編集中に
配置や回転のミスを発見したりできます。以下に例を示します:
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
        シンチレーターバレル
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
        ブーリアン演算による立体
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

リンクを開くと、GEMC のサンプルから生成されたインタラクティブな PyVista シーンを確認できます。


<br/>

## シミュレーションの実行

GEMC は YAML ファイルと、それに相当するコマンドラインオプションで設定します。簡潔なステアリングファイルで、
ラン番号、イベント数、ジェネレーター、ジオメトリシステム、出力ストリーマー、ワールドボリュームを定義できます:

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

Geant4 マクロのテキストファイルを実行（次回リリースで導入予定）:

```shell
gemc counter.yaml -geant4_macro=run.mac
gemc counter.yaml -gui -geant4_macro=vis.mac
```

これに相当する YAML の設定は `geant4_macro: run.mac` です。GEMC は初期化と可視化のセットアップの後、
GUI または対話型ターミナルに入る前にマクロを実行します。相対パスは作業ディレクトリを基準に解決されます。
Geant4 はコメント、エイリアス、ネストされた `/control/execute` コマンドを処理します。ファイルが見つからない場合や
コマンドが失敗した場合は、非ゼロの終了ステータスが返されます。

バッチモードでは、マクロが `/run/beamOn 100` などのコマンドでイベント生成を制御します。GEMC はその後に、
`-n` で指定されたイベント数や `-run_weights` による割り当てに従って自動実行することはありません。
検出器の条件は `-run` を使って準備されます。可視化専用のマクロは、ビームを開始するコマンドを含まない限り、
イベントを生成しません。`geant4_macro` がない場合、通常のイベント処理は変わりません。

オフスクリーンの Geant4 画像を生成:

```shell
gemc counter.yaml -n=10 \
  -g4view="[{driver: TOOLSSG_OFFSCREEN, segsPerCircle: 200}]" \
  -g4camera="[{phi: -10*deg, theta: 250*deg}]" \
  -g4light="[{phi: 160*deg, theta: 120*deg}]"
```

<br/>

## 出力と解析

出力レイヤーはプラグインベースです。組み込みのストリーマー形式は次のとおりです:

| 形式    | 備考                                                                                                             |
|---------|------------------------------------------------------------------------------------------------------------------|
| `ascii` | 人間が読めるテキスト出力                                                                                           |
| `csv`   | デジタイズされたヒット、真値情報、生成粒子、追跡された生成粒子についてのスレッドごとの CSV テーブル                    |
| `json`  | 構造化されたイベント出力                                                                                           |
| `root`  | ROOT TTree。GEMC が ROOT を有効にしてビルドされている場合に利用可能                                                    |

次回リリースでは、イベント出力にトラックの由来情報を含める機能が導入される予定です:

```shell
gemc counter.yaml -save_original_track
gemc counter.yaml -save_all_ancestors
```

`-save_original_track` は、各ヒットの真値情報の `otid` に値を設定します。`-save_all_ancestors` は
元トラック ID も有効にし、ヒットを生成した各トラックとその親をたどる完全な連鎖を含む、重複を除去した
`ancestors` バンクを書き込みます。CSV 出力では別個の `_ancestors.csv` ファイルを使用します。
ASCII、JSON、ROOT では、このバンクをイベント出力に含めます。


`pygemc` は CSV または ROOT 出力を解析するためのユーティリティを提供します。例:

```shell
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
gemc-analyzer out.root E --kind root --detector flux --save energy.png
```


たとえば、[`b2` のサンプル](https://gemc.github.io/home/examples/basic/b2/)を
`gemc b2.yaml -n=1000` で実行した後、アナライザーは次のプロットを生成します:

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
- [C++ ロギング](https://gemc.github.io/home/documentation/api/logging/) — 番号付きメッセージテンプレート、
  フィルタリング、エラー処理。テンプレートメソッドは次回リリースで導入予定
- [スレッドスケーリング](https://gemc.github.io/home/documentation/support/thread_scaling) — マルチスレッドの
  ベンチマークと、スレッド数を変えたスケーリング測定
- [プロファイリング](https://gemc.github.io/home/documentation/support/valgrind_profile) — Valgrind callgrind の指標
  とその読み方
- [Python API リポジトリ](https://github.com/gemc/pygemc)
- [CLAS12 GEMC システムリポジトリ](https://github.com/gemc/clas12-systems)


<br/>

## コントリビューション

通常のプルリクエストによる貢献を歓迎します。プルリクエストを作成する前に、関連する meson テストを実行し、
変更範囲を絞ってください。以下を参照してください:

- [`CONTRIBUTING.md`](CONTRIBUTING.md)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md)
- [`SECURITY.md`](SECURITY.md)
- [CI ワークフローガイド](.github/workflows/README.md)

次回リリースで導入予定: C++ ロギングメソッドは、番号付きパラメーターを含む完全なメッセージを受け取ります:

```cpp
log->info(0, "Using %1 threads out of %2 available cores.", useThreads, ncores);
```

外部プラグインで文字列を連結している呼び出しを新しい形式に移行し、対応する GEMC ヘッダーを使って再ビルドしてください。
移行期間用の `*Template()` メソッドは、引き続き呼び出しを転送するエイリアスとして残ります。
プレースホルダーのルールと動的な診断テキストの扱いについては、
[ロギングガイド](https://gemc.github.io/home/documentation/api/logging/)を参照してください。

英語のロギングカタログは、コードを管理する各リポジトリ内の C++ テンプレートから生成されます。
抽出、検証、インストールのコマンドについては[メッセージカタログ](translations/README.md)を参照してください。
ロギング機能は言語を自動検出します。`-language=it` はイタリア語を選択し、`-language=en` は英語を強制します。
翻訳がない場合は英語にフォールバックします。共有の Claude/Codex ワークフローは、ウェブサイトと README の
翻訳に使用するものと同じ平日の cron ジョブを通じて、プロジェクトの各言語のカタログを生成します。

次回リリースで導入予定: GUI コントロール、ツールチップ、ジオメトリの説明、アナライザーのメッセージ、
SVG ボタンのキャプションに同じ言語設定が適用されます。[GUI 翻訳](translations/gui/README.md)を参照してください。
Qt Linguist ツールがビルド時に GUI カタログをコンパイルします。標準ダイアログの翻訳には、
Qt にインストールされている言語データを使用します。

<br/>

## 引用

GEMC を科学研究で使用する場合は、次の文献を引用してください:

> M. Ungaro and the CLAS Collaboration, "The CLAS12 Geant4 simulation," *Nuclear Instruments and Methods
> in Physics Research Section A* **959**, 163422 (2020). https://doi.org/10.1016/j.nima.2020.163422

BibTeX およびその他の引用形式は [`CITATION.md`](CITATION.md) にあります。

<br/>

## ライセンス

GEMC は [Apache ライセンス、バージョン 2.0](LICENSE) に基づいてライセンスされています。
帰属表示および第三者への謝辞については [`NOTICE`](NOTICE) を参照してください。本ソフトウェアは、
Geant4、CLHEP、Qt、ROOT、SQLite、Assimp など、個別のライセンスが適用される第三者コンポーネントにも依存しています。

<br/>

## ロードマップ

現在の開発計画については、[プロジェクトロードマップ](https://github.com/orgs/gemc/projects/1/views/4)を参照してください。
