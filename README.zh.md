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

**GEMC** 是一款数据库驱动的 [Geant4](https://geant4.web.cern.ch) 应用程序，用于探测器与
辐射输运模拟。借助其 Python API，你无需编写 Geant4 代码即可定义复杂的探测器装置。

探测器描述与可执行文件保持分离：你使用 Python 创建探测器描述，将其存储到
ASCII 或 SQLite 数据库中，GEMC 在运行时加载这些描述以执行 Geant4 模拟。

C++ 插件支持自定义探测器响应、磁场、事件产生器和输出格式。

![GEMC 架构](https://gemc.github.io/home/assets/images/gemcArchitecture.svg)

<br/>

## 亮点

- 以 Python 为主要接口的探测器定义
- 使用 [`pyvista`](https://pyvista.org) 进行几何预览和 VTK 导出
- ASCII、SQLite 数据库
- 从 GDML 和 CAD 文件导入几何
- 交互式 Geant4/Qt 可视化及离屏图像生成
- 内置灵敏类型：`flux`、`gPhotonDetector`、`dosimeter`、`particle_counter`
- 支持 ASCII、CSV、JSON 和 [ROOT](https://root.cern) 格式的模拟输出
- 支持通过 GUI、命令行、YAML 和 LUND 文件产生事件
- 磁场支持，包括多极磁场和 ASCII 磁场图
- 用于读取并绘制模拟输出（CSV、ROOT）的 Python `analyzer` 模块
- 几何变体与随运行号变化的配置
- 基于 meson 的 C++ 构建，并为 Linux `amd64` 和 `arm64` 提供经 CI 测试的 Docker 镜像


<br/>

## 快速上手

试用 GEMC 最快捷的方式是通过示例：

- [Jupyter 快速上手](https://mybinder.org/v2/gh/gemc/binder-tutorials/main?urlpath=lab/tree/notebooks/basic/quickstart.ipynb)
- [文档与示例](https://gemc.github.io/home/)
- [示例画廊](https://gemc.github.io/home/examples/)

以下最简本地工作流创建一个 `counter` 探测器，并使用它模拟 1000 个事件：

```shell
gemc-system-template -s counter
cd counter
./counter.py
gemc counter.yaml -n=1000
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
```

该模板命令创建一个 `counter` 系统，
Python 脚本构建几何并将其存储到 SQLite 文件 `gemc.db` 中，
`gemc` 运行模拟，`gemc-analyzer` 则绘制 CSV 输出中的某个变量。

<br/>

## 安装

安装说明请参见[安装指南](https://gemc.github.io/home/installation/)


<br/>

## pygemc

探测器系统使用 [`pygemc`](https://pypi.org/project/pygemc/) 构建并写入数据库。
以下是定义一个用于通量探测的灵敏盒体的示例：

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

几何脚本可以写入 SQLite 或 ASCII 数据库，使用 PyVista 显示几何，或导出 VTK.js 文件：

```shell
./counter.py --factory sqlite
./counter.py -pv
./counter.py -pvvtk counter -pvz 0.25
```

<br/>

### PyVista 可视化

PyVista 是 GEMC 常规几何工作流的一部分。用户可以在运行 Geant4 之前检查探测器几何，
导出可移植的 VTK.js 场景用于文档，并在编辑 Python 几何脚本时发现位置或旋转错误。
以下是一些示例：
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
        闪烁体桶部
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

点击链接，打开从 GEMC 示例生成的交互式 PyVista 场景。


<br/>

## 运行模拟

GEMC 通过 YAML 文件及等效的命令行选项进行配置。一个简洁的运行控制文件可以定义运行号、
事件数、产生器、几何系统、输出流处理器以及世界体：

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

执行一个 Geant4 宏文本文件（将在下一版本中推出）：

```shell
gemc counter.yaml -geant4_macro=run.mac
gemc counter.yaml -gui -geant4_macro=vis.mac
```

等效的 YAML 设置为 `geant4_macro: run.mac`。GEMC 在初始化和可视化设置之后、
进入 GUI 或交互式终端之前执行该宏。相对路径以工作目录为基准；
Geant4 会处理注释、别名以及嵌套的 `/control/execute` 命令。文件缺失和命令失败会产生非零退出状态。

在批处理模式下，宏通过诸如 `/run/beamOn 100` 之类的命令控制事件产生；GEMC 之后不会
自动运行 `-n` 指定的事件，也不会执行 `-run_weights` 指定的事件分配。探测器条件使用
`-run` 来准备。仅用于可视化的宏除非包含启动束流的命令，否则不产生任何事件。若未设置
`geant4_macro`，常规的事件处理保持不变。

生成一张离屏 Geant4 图像：

```shell
gemc counter.yaml -n=10 \
  -g4view="[{driver: TOOLSSG_OFFSCREEN, segsPerCircle: 200}]" \
  -g4camera="[{phi: -10*deg, theta: 250*deg}]" \
  -g4light="[{phi: 160*deg, theta: 120*deg}]"
```

<br/>

## 输出与分析

输出层基于插件。内置的流处理器格式包括：

| 格式    | 说明                                                                                                             |
|---------|------------------------------------------------------------------------------------------------------------------|
| `ascii` | 人类可读的文本输出                                                                                               |
| `csv`   | 按线程生成的 CSV 表，包含数字化击中、真实信息、产生的粒子以及经过跟踪的产生粒子                                     |
| `json`  | 结构化的事件输出                                                                                                 |
| `root`  | ROOT TTree，在构建 GEMC 时启用 ROOT 支持后可用                                                                     |

将在下一版本中推出：可为事件输出启用径迹溯源：

```shell
gemc counter.yaml -save_original_track
gemc counter.yaml -save_all_ancestors
```

`-save_original_track` 会在每条击中的真实信息记录中填充 `otid`。`-save_all_ancestors` 还会启用
原始径迹 ID，并写入一个去重的 `ancestors` 数据组，其中包含每条产生击中的径迹及其
完整的父径迹链。CSV 输出使用单独的 `_ancestors.csv` 文件；ASCII、JSON 和 ROOT 则在
各自的事件输出中包含该数据组。


`pygemc` 提供了一些用于分析 CSV 或 ROOT 输出的实用工具。例如：

```shell
gemc-analyzer counter_t0_digitized.csv totEdep --kind csv --bins 50
gemc-analyzer out.root E --kind root --detector flux --save energy.png
```


例如，使用 [`b2` 示例](https://gemc.github.io/home/examples/basic/b2/)运行
`gemc b2.yaml -n=1000` 后，分析器会生成：

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
- [C++ 日志](https://gemc.github.io/home/documentation/api/logging/) —— 带编号参数的消息模板、
  过滤和错误处理；模板方法将在下一版本中推出
- [线程扩展性](https://gemc.github.io/home/documentation/support/thread_scaling) —— 多线程基准测试
  与线程扩展性扫描测试
- [性能剖析](https://gemc.github.io/home/documentation/support/valgrind_profile) —— Valgrind callgrind 指标
  及其解读方法
- [Python API 仓库](https://github.com/gemc/pygemc)
- [CLAS12 GEMC 系统仓库](https://github.com/gemc/clas12-systems)


<br/>

## 贡献

欢迎通过常规的拉取请求（pull request）参与贡献。在发起拉取请求之前，请运行相关的 meson 测试，
并使改动集中于当前目标。参见：

- [`CONTRIBUTING.md`](CONTRIBUTING.md)
- [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md)
- [`SECURITY.md`](SECURITY.md)
- [CI 工作流指南](.github/workflows/README.md)

将在下一版本中推出：C++ 日志方法接受带编号参数的完整消息：

```cpp
log->info(0, "Using %1 threads out of %2 available cores.", useThreads, ncores);
```

请迁移外部插件中使用字符串拼接的调用，并使用匹配的 GEMC 头文件重新构建。过渡性的
`*Template()` 方法仍作为转发别名保留。占位符规则以及动态诊断文本的处理方法请参见
[日志指南](https://gemc.github.io/home/documentation/api/logging/)。

英文日志消息集由各自所属仓库中的 C++ 模板生成。提取、验证和安装命令请参见
[消息集](translations/README.md)。日志会自动检测语言；`-language=it` 选择意大利语，
`-language=en` 强制使用英语。缺失的翻译会回退到英语。共享的 Claude/Codex 工作流
通过与网站和 README 翻译相同的工作日 cron 任务，生成项目所用语言的消息集。

将在下一版本中推出：GUI 控件、工具提示、几何描述、分析器消息以及 SVG 按钮文字
都使用相同的语言选择。参见 [GUI 翻译](translations/gui/README.md)。Qt Linguist 工具
在构建期间编译 GUI 消息集；标准对话框的翻译使用 Qt 已安装的语言数据。

<br/>

## 引用

如果你在科研工作中使用了 GEMC，请引用：

> M. Ungaro and the CLAS Collaboration, "The CLAS12 Geant4 simulation," *Nuclear Instruments and Methods
> in Physics Research Section A* **959**, 163422 (2020). https://doi.org/10.1016/j.nima.2020.163422

BibTeX 及其他引用格式见 [`CITATION.md`](CITATION.md)。

<br/>

## 许可证

GEMC 采用 [Apache 许可证 2.0 版](LICENSE)；归属声明与第三方致谢见 [`NOTICE`](NOTICE)。
本软件还依赖于单独授权的第三方组件，包括
Geant4、CLHEP、Qt、ROOT、SQLite 和 Assimp。

<br/>

## 路线图

当前开发计划请参见[项目路线图](https://github.com/orgs/gemc/projects/1/views/4)。
