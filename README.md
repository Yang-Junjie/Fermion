# Fermion

Fermion 是一款基于 Hazel2D 扩展而来的的轻量级游戏引擎。

## 命名哲学

- **Fermion（费米子）**：对应引擎核心。费米子是构成物质的基本粒子，象征核心运行时负责承载游戏世界中的一切对象与逻辑。
- **Boson（玻色子）**：对应编辑器。玻色子是传递费米子之间的相互作用的媒介粒子，象征编辑器作为开发者与引擎之间的“交互媒介”，用于搭建场景、调整参数、驱动物体。
- **Higgs（希格斯粒子）**：对应物理模块。希格斯场赋予粒子质量，物理系统则为引擎中的对象带来“重量”“惯性”和真实的运动行为。
- **Photon（光子）**: 对应脚本系统。光子是一种玻色子，标志着Photoon是编辑器的一部分。并且光子轻、快象征着脚本的轻、快。


## 设计目标
- 用于锻炼我自己的编程/架构/图形/抽象能力的游戏引擎
- 开发者通过插件机制对 Boson 编辑器进行定制或功能扩展，将其转变为适配特定需求的专用工具。
- 用户能够轻松的使用脚本或者蓝图创建游戏
- 打造一款简单易用的扩展性强的通用跨平台游戏引擎


## 核心特性

- **C++20 / 模块化架构**
  - 基于 `Engine` + `Layer` 的应用框架和图层系统
  - 自定义事件系统（窗口、键盘、鼠标、应用生命周期）
  - 时间与帧步进管理（`Timestep`、`Timer`）

- **2D 渲染管线（OpenGL）**
  - 通过 `RendererAPI` / `RenderCommand` 抽象渲染后端，目前实现为 OpenGL + GLAD
  - `Renderer2D` 支持批量绘制：
    - 纯色/纹理矩形、旋转矩形
    - 基于纹理图集的 `SubTexture2D`
    - 圆形、线段与矩形轮廓（用于调试或 Gizmo）
  - `SceneRenerer` 封装渲染指令用于场景渲染
  - 帧缓冲（`Framebuffer`）用于 Boson 视口渲染与离屏绘制
  - 多种相机类型：`OrthographicCamera`、`SceneCamera`、`EditorCamera`

- **实体组件系统（ECS）**
  - 基于 [entt](https://github.com/skypjack/entt) 的 `entt::registry`
  - 常用内置组件：
    - `IDComponent`、`TagComponent`
    - `TransformComponent`（平移、欧拉旋转、缩放）
    - `SpriteRendererComponent`、`CircleRendererComponent`、`TextComponent`
    - `CameraComponent`
    - `NativeScriptComponent`（C++ 行为脚本）
    - `Rigidbody2DComponent`、`BoxCollider2DComponent`、`CircleCollider2DComponent`
  - `Entity` 封装组件增删查；`ScriptableEntity` 提供脚本访问实体组件的入口

- **2D 物理（Box2D 集成）**
  - 使用新版 [box2d](https://github.com/erincatto/box2d) API
  - 在 `Scene::onRuntimeStart` 中构建物理世界，按组件自动生成刚体与碰撞体
  - 在 `Scene::onUpdateRuntime` 中同步 Box2D 世界状态到实体 Transform
  - 支持刚体类型（静态/动态/运动学）、固定旋转、矩形/圆形碰撞体及物理材质参数

- **场景与序列化**
  - `Scene` 支持 Editor/Runtime 两种更新模式
  - 通过 [yaml-cpp](https://github.com/jbeder/yaml-cpp) 将场景序列化为可读的 `.fermion` YAML 文件
  - `SceneSerializer` 负责读写实体及其组件（Transform、渲染、相机、物理等）

- **ImGui 工具与调试**
  - `ImGuiLayer` 集成 Dear ImGui + docking + 多视口
  - 使用 [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) 实现编辑器中的变换 Gizmo
  - 示例层（如 `SandBox2D`）提供绘制统计面板、参数调节等调试 UI

- **Boson 编辑器**
  - 使用 Fermion 引擎实现的独立可执行程序 `BosonEditor`
  - 核心功能：
    - 视口窗口：使用 `EditorCamera` 查看/编辑场景
    - 场景层级面板（`SceneHierarchyPanel`）：编辑实体、添加/删除组件、修改组件属性
    - 资源浏览器（`ContentBrowserPanel`）：浏览 `assets` 目录中的资源
    - 播放/停止按钮切换编辑模式与运行时模拟
    - 显示或隐藏物理碰撞体轮廓
  - 场景文件示例位于 `Boson/assets/scenes/*.fermion`

- **示例游戏（game）**
  - `game` 目录包含基于 Fermion 的示例应用：
    - `SandBox2D` 展示 2D 渲染、相机控制、纹理/图集绘制以及渲染统计
    - `GameLayer` 示例如何在自定义图层中使用引擎 API

- **日志与性能分析**
  - 日志系统基于 [spdlog](https://github.com/gabime/spdlog)，由 `Core/Log` 封装
  - `Instrumentor` / `InstrumentationTimer` 支持将性能数据导出为 Chrome Trace 格式 JSON 文件（可选启用）

## 目录结构概览

```text
Fermion/
├── Fermion/                 # 引擎核心
│   ├── Sources/
│   │   ├── Core/            # Engine、Layer、Window、日志、时间
│   │   ├── Events/          # 事件系统
│   │   ├── Renderer/        # Renderer2D、Shader、Texture、Camera 等
│   │   ├── Scene/           # Scene、Entity、组件、场景序列化
│   │   ├── Physics/         # Box2D 2D 物理封装
│   │   ├── Project/         # 项目管理
│   │   ├── ImGui/           # ImGui 层与主题
│   │   ├── Math/            # 变换分解等数学工具
│   │   ├── Time/            # 计时器
│   │   └── Utils/           # 平台工具（文件对话框、时间）
│   └── Platform/
│       ├── OpenGL/          # OpenGL 渲染后端实现
│       └── Window/          # 基于 GLFW 的窗口与输入适配
├── Boson/                   # Boson 编辑器
│   ├── Panels/              # 场景层级面板、内容浏览器面板
│   ├── assets/              # 编辑器资源与示例场景
│   └── Resources/           # 图标等资源
├── game/                    # 示例游戏工程
│   └── assets/              # 示例用贴图与着色器
├── external/                # 第三方依赖（通过 submodule 或源码集成）
└── CMakeLists.txt           # 顶层 CMake 配置
```

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
- [Mono](https://github.com/mono/mono.git) - 跨平台 .NET 运行时，用于在引擎中运行 C# 脚本。

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
cmake .. -DUSE_GLFW=ON -DBUILD_GAME=ON -DBUILD_BOSON=ON
cmake --build . --config Release
```

构建完成后，可执行文件默认输出到根目录下的 `bin/` 目录：

- `bin/BosonEditor[.exe]` – 场景编辑器
- `bin/game[.exe]` – 示例游戏

### 运行

在项目根目录或 `bin/` 目录下运行：

```bash
# 编辑器
./bin/BosonEditor

# 示例游戏
./bin/game
```

请确保运行时工作目录能正确访问 `Boson/assets` 与 `game/assets`，否则字体/纹理等资源可能无法加载。

## 快速上手示例

使用 Fermion 编写一个最小示例应用：

```cpp
#include "Fermion.hpp"

class ExampleLayer : public Fermion::Layer {
public:
    ExampleLayer() : Layer("ExampleLayer") {}

    void onUpdate(Fermion::Timestep dt) override {
        Fermion::RenderCommand::setClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        Fermion::RenderCommand::clear();

        Fermion::Renderer2D::beginScene(m_camera);
        Fermion::Renderer2D::drawQuad({0.0f, 0.0f}, {1.0f, 1.0f}, {0.2f, 0.8f, 0.3f, 1.0f});
        Fermion::Renderer2D::endScene();
    }

private:
    Fermion::OrthographicCamera m_camera{-1.6f, 1.6f, -0.9f, 0.9f};
};

class ExampleApp : public Fermion::Engine {
public:
    ExampleApp() : Engine("Example") {
        pushLayer(std::make_unique<ExampleLayer>());
    }
};

// 应用入口：必须在客户端实现
Fermion::Engine* Fermion::createEngine() {
    return new ExampleApp();
}
```

## 许可证

本项目基于 MIT License 开源，详情请见 `LICENSE` 文件。

