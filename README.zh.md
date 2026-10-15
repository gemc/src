<!-- TRANSLATIONS:START (managed by bin/translate) -->
**Languages:** [EN](README.md) · [IT](README.it.md) · [日本語](README.ja.md) · [FR](README.fr.md) · **中文**
<!-- TRANSLATIONS:END -->

# GEMC

<table>
  <thead>
    <tr>
      <th width="33%">质量</th>
      <th width="34%">部署</th>
      <th width="33%">文档</th>
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

GEMC（GEant Monte-Carlo）是一个数据库驱动的 [Geant4](https://geant4.web.cern.ch) 应用，用于探测器和
辐射输运
模拟。它将探测器描述保存在已编译的 C++ 应用之外：

- 用户在 Python 中构建探测器装置
- GEMC 在运行时加载这些定义，并执行 Geant4 模拟流程

其目标是让希望对探测器系统进行原型设计的用户能够方便地使用基于 Geant4 的模拟，
而无需编写自定义 Geant4 应用，
同时仍为高级探测器响应和输出工作流保留 C++ 扩展点。

![GEMC 架构](https://gemc.github.io/home/assets/images/gemcArchitecture.svg)

<br/>

## 亮点

- 通过 [`pygemc`](https://github.com/gemc/pygemc) 以 Python 为主要方式定义探测器
- 几何与材料存储于 SQLite 或 ASCII 数据库
- 从 GDML、CAD 网格导入几何
- 运行编号与几何变体
- 内置灵敏探测器类型：`flux`、`gPhotonDetector`、`dosimeter`、`particle_counter`
- 用于自定义数字化、场、生成器和输出流处理器的 C++ 插件基础设施
- 通过命令行/YAML 粒子定义以及 LUND 文件进行事例生成
- 支持的输出格式：ASCII、CSV、JSON、ROOT
- 磁场支持，包括多极磁场和 ASCII 场图
- 交互式 Geant4/Qt 可视化及离屏图像生成
- PyVista 几何预览、VTK 导出
- Python 分析器，支持 CSV 和 ROOT 格式
- 基于 Meson 的 C++ 构建，并为 Linux `amd64` 和 `arm64` 提供经 CI 测试的 Docker 镜像

<br/>

## 快速上手

试用 GEMC 最快的方式是通过示例：

- [Jupyter 快速上手](https://mybinder.org/v2/gh/gemc/binder-tutorials/main?urlpath=lab/tree/notebooks/basic/quickstart.ipynb)
- [文档与示例](https://gemc.github.io/home/)
- [示例库](https://gemc.github.io/home/examples/)

以下最小本地工作流创建一个 `counter` 探测器，并在其中模拟 1000 个事例：

```shell
gemc-system-template -s counter
cd counter
./counter.py
gemc counter.yaml -n=1000
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
```

模板命令会创建一个 `counter` 系统，
Python 脚本构建几何并将其存入 SQLite 文件 `gemc.db`，
`gemc` 运行模拟，`gemc-analyzer` 则根据 CSV 输出绘制某个变量的分布。

<br/>

## 安装

安装说明请参见[安装指南](https://gemc.github.io/home/installation/)


<br/>

## pygemc

探测器系统使用 [`pygemc`](https://pypi.org/project/pygemc/) 构建并写入数据库。
以下是一个定义灵敏通量盒的示例：

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

几何脚本可以写入 SQLite 或 ASCII 数据库、使用 PyVista 显示几何，或导出 VTK.js 文件：

```shell
./counter.py --factory sqlite
./counter.py -pv
./counter.py -pvvtk counter -pvz 0.25
```

<br/>

### PyVista 可视化

PyVista 是 GEMC 常规几何工作流的一部分。它让用户能在运行 Geant4 之前检查探测器几何，
导出可移植的 VTK.js 场景用于文档，并在编辑 Python 几何脚本时发现
位置或旋转设置错误。以下是一些示例：
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
        桶形闪烁体
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
        布尔实体
      </a>
    </td>
    <td>
      <a href="https://gemc.github.io/home/assets/vtkjs-viewer.html?fileURL=/home/assets/images/examples/cad/cad.vtksz"
         target="_blank" rel="noopener noreferrer">
        <img src="https://gemc.github.io/home/assets/images/examples/cad/gemc_view.png"
             alt="CAD Import PyVista" width="180"><br>
        CAD 导入
      </a>
    </td>
  </tr>
</table>

点击链接，打开由 GEMC 示例生成的交互式 PyVista 场景。


<br/>

## 运行模拟

GEMC 通过 YAML 文件以及等效的命令行选项进行配置。一个简洁的运行控制文件可以定义运行
编号、事例数、生成器、几何系统、输出流处理器和世界体积：

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

以批处理模式运行：

```shell
gemc counter.yaml
```

使用 Geant4 GUI 运行：

```shell
gemc counter.yaml -gui
```

执行 Geant4 宏文本文件（将在下个版本中推出）：

```shell
gemc counter.yaml -geant4_macro=run.mac
gemc counter.yaml -gui -geant4_macro=vis.mac
```

等效的 YAML 设置为 `geant4_macro: run.mac`。GEMC 会在初始化和可视化设置之后、
进入 GUI 或交互式终端之前执行该宏。相对路径以工作目录为基准；
Geant4 会处理注释、别名以及嵌套的 `/control/execute` 命令。文件缺失和命令失败
会导致非零退出状态。

在批处理模式下，宏通过诸如 `/run/beamOn 100` 之类的命令控制事例生成；GEMC 不会
在其后自动处理 `-n` 指定的事例或按 `-run_weights` 分配的事例。探测器条件通过
`-run` 准备。仅用于可视化的宏除非包含启动束流的命令，否则不会生成任何事例。未设置
`geant4_macro` 时，常规事例处理保持不变。

生成离屏 Geant4 图像：

```shell
gemc counter.yaml -n=10 \
  -g4view="[{driver: TOOLSSG_OFFSCREEN, segsPerCircle: 200}]" \
  -g4camera="[{phi: -10*deg, theta: 250*deg}]" \
  -g4light="[{phi: 160*deg, theta: 120*deg}]"
```

<br/>

## 输出与分析

输出层基于插件。内置的输出流处理器格式包括：

| 格式    | 说明                                                                                                             |
|---------|------------------------------------------------------------------------------------------------------------------|
| `ascii` | 人类可读的文本输出                                                                                               |
| `csv`   | 按线程分别输出的 CSV 表，包含数字化击中、真实信息、生成粒子以及被跟踪的生成粒子                                     |
| `json`  | 结构化事例输出                                                                                                   |
| `root`  | ROOT TTree，在使用 ROOT 构建 GEMC 时可用                                                                           |

将在下个版本中推出：可为事例输出启用径迹溯源：

```shell
gemc counter.yaml -save_original_track
gemc counter.yaml -save_all_ancestors
```

`-save_original_track` 会在每条真实信息击中记录中填充 `otid`。`-save_all_ancestors` 还会启用
原始径迹 ID，并写入一个去重后的 `ancestors` 数据组，其中包含每条产生击中的径迹及其
完整的父径迹链。CSV 输出使用单独的 `_ancestors.csv` 文件；ASCII、JSON 和 ROOT 则将该数据组
包含在各自的事例输出中。


`pygemc` 提供了一些用于分析 CSV 或 ROOT 输出的实用工具。例如：

```shell
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
gemc-analyzer out.root E --kind root --detector flux --save energy.png
```


例如，在运行 [`b2` 示例](https://gemc.github.io/home/examples/basic/b2/) 并使用
`gemc b2.yaml -n=1000` 命令之后，分析器会生成：

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

## 文档

- [GEMC 主页](https://gemc.github.io/home/)
- [C++ 日志](https://gemc.github.io/home/documentation/api/logging/) — 带编号占位符的消息模板、
  过滤和错误处理；模板方法将在下个版本中推出
- [线程扩展性](https://gemc.github.io/home/documentation/support/thread_scaling) — 多线程基准测试
  与扩展性扫描测试
- [性能剖析](https://gemc.github.io/home/documentation/support/valgrind_profile) — Valgrind callgrind 指标
  及其解读方式
- [Python API 仓库](https://github.com/gemc/pygemc)
- [CLAS12 GEMC 系统仓库](https://github.com/gemc/clas12-systems)


<br/>

## 贡献

欢迎通过常规的拉取请求进行贡献。在提交拉取请求之前，请运行相关的 Meson 测试，
并保持改动集中。请参见：

- [`CONTRIBUTING.md`](CONTRIBUTING.md)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md)
- [`SECURITY.md`](SECURITY.md)
- [CI 工作流指南](.github/workflows/README.md)

将在下个版本中推出：C++ 日志方法接受带编号参数的完整消息：

```cpp
log->info(0, "Using %1 threads out of %2 available cores.", useThreads, ncores);
```

迁移外部插件中通过字符串拼接构造消息的日志调用，并使用匹配的 GEMC 头文件重新构建。过渡性的
`*Template()` 方法仍作为转发别名保留。有关占位符规则以及
动态诊断文本的处理，请参见
[日志指南](https://gemc.github.io/home/documentation/api/logging/)。

英文日志消息目录由各所属仓库中的 C++ 模板生成。有关提取、校验和安装命令，请参见
[消息目录](translations/README.md)。日志系统会自动检测语言；`-language=it` 选择意大利语，
`-language=en` 强制使用英语。缺失的翻译会回退到英语。共享的 Claude/Codex 工作流
通过与网站和 README 翻译共用的工作日 cron 作业，
生成项目各语言的消息目录。

将在下个版本中推出：GUI 控件、工具提示、几何描述、分析器消息以及 SVG 按钮文字
都使用相同的语言选择。请参见 [GUI 翻译](translations/gui/README.md)。Qt Linguist 工具
会在构建期间编译 GUI 翻译目录；标准对话框的翻译使用 Qt 已安装的语言数据。

<br/>

## 引用

如果您在科学研究中使用 GEMC，请引用：

> M. Ungaro and the CLAS Collaboration, "The CLAS12 Geant4 simulation," *Nuclear Instruments and Methods
> in Physics Research Section A* **959**, 163422 (2020). https://doi.org/10.1016/j.nima.2020.163422

BibTeX 及其他引用格式见 [`CITATION.md`](CITATION.md)。

<br/>

## 许可证

GEMC 采用 [Apache 许可证 2.0 版](LICENSE) 授权；有关归属声明和
第三方致谢，请参见 [`NOTICE`](NOTICE)。本软件还依赖于采用各自许可证的第三方组件，包括
Geant4、CLHEP、Qt、ROOT、SQLite 和 Assimp。

<br/>

## 路线图

当前的开发计划请参见[项目路线图](https://github.com/orgs/gemc/projects/1/views/4)。
