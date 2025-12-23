# Fermion

Fermion 是一款基于 Hazel2D 扩展而来的的轻量级游戏引擎。

## 命名哲学

- **Fermion（费米子）**：对应引擎核心。费米子是构成物质的基本粒子，象征核心运行时负责承载游戏世界中的一切对象与逻辑。
- **Boson（玻色子）**：对应编辑器。玻色子是传递费米子之间的相互作用的媒介粒子，象征编辑器作为开发者与引擎之间的“交互媒介”，用于搭建场景、调整参数、驱动物体。
- **Higgs（希格斯粒子）**：对应物理模块。希格斯场赋予粒子质量，物理系统则为引擎中的对象带来“重量”“惯性”和真实的运动行为。
- **Photon（光子）**: 对应脚本系统。光子是一种玻色子，标志着Photoon是编辑器的一部分。并且光子轻、快象征着脚本的轻、快。
- **Neutrino（中微子）**：对应引擎的运行时(Runtime)。中微子是一种费米子，标志着运行时是引擎的一部分。并且中微子几乎不与物质相互作用，就像运行时不直接被玩家看见负责维持内部机制正常运转。
- **HilbertSpace(希艾伯特空间)** :插件系统

## 设计目标
- 用于锻炼我自己的编程/架构/图形/抽象能力的游戏引擎
- 开发者通过插件机制对 Boson 编辑器进行定制或功能扩展，将其转变为适配特定需求的专用工具。
- 用户能够轻松的使用脚本或者蓝图创建游戏
- 打造一款简单易用的扩展性强的通用跨平台游戏引擎


## 第三方依赖

项目依赖的库均以源码形式引入，便于跨平台编译与调试：

- [spdlog](https://github.com/gabime/spdlog) – 高性能日志库
- [entt](https://github.com/skypjack/entt) – 实体组件系统（ECS）
- [glm](https://github.com/g-truc/glm) – 数学库（向量、矩阵、变换）
- [Dear ImGui](https://github.com/ocornut/imgui) – 即时模式图形界面
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) – 编辑器中的变换 Gizmo
- [GLFW](https://github.com/glfw/glfw) – 跨平台窗口与输入管理
- [GLAD](https://glad.dav1d.de/) – OpenGL 函数加载器
- [stb](https://github.com/nothings/stb) – 使用 `stb_image` 进行纹理加载
- [yaml-cpp](https://github.com/jbeder/yaml-cpp) – YAML 序列化（场景保存/加载）
- [box2d](https://github.com/erincatto/box2d) – 2D 刚体物理引擎
- [msdf-atlas-gen](https://github.com/Chlumsky/msdf-atlas-gen) - MSDF 纹理生成
- [freetype](https://github.com/freetype/freetype) - FreeType 字体库
- [Mono](https://github.com/mono/mono) - 跨平台 .NET 运行时，用于在引擎中运行 C# 脚本。
- [Assimp](https://github.com/assimp/assimp) - 3D 模型加载

## 构建说明

### 环境要求

- CMake ≥ 3.16
- 支持 C++20 的编译器
  - Windows：Visual Studio 2019 或更新版本
  - 其他平台：GCC / Clang（需自行提供 `PlatformUtils` 等平台适配实现，当前工程主要在 Windows 下开发与测试）
- 已安装 Git（用于克隆子模块）

### 获取源码

```bash
git clone https://github.com/Yang-Junjie/Fermion.git
cd Fermion
git submodule update --init --recursive
```

### 配置与编译

```bash
mkdir build
cd build
cmake .. 
cmake --build . --config Release
```

## 许可证

本项目基于 MIT License 开源，详情请见 `LICENSE` 文件。
