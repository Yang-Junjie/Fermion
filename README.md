# Fermion

Fermion 是一款基于 C++20 开发的轻量级游戏引擎。引擎采用模块化设计，支持多渲染后端，提供简洁的 API 以简化游戏开发流程。



## 特性

- **跨平台支持**：通过抽象层支持多种渲染后端
- **模块化架构**：清晰的代码组织和分层设计
- **事件系统**：灵活的事件处理机制
- **图层系统**：基于 Layer 的场景管理
- **渲染功能**：基于 OpenGL 的图形渲染
- **日志系统**：集成 spdlog 提供详细日志
- **实体组件系统**：基于 entt 实现
- **ImGui 集成**：内置 ImGui 用于调试和界面开发
- **窗口管理**：基于 GLFW 的跨平台窗口系统

## 技术栈与依赖

- **编程语言**：C++20
- **构建系统**：CMake 3.16+
- **主要依赖**：
  - [OpenGL](https://www.opengl.org/) - 图形渲染 API
  - [GLFW](https://www.glfw.org/) - 跨平台窗口和输入处理
  - glad - OpenGL 加载库
  - [ImGui](https://github.com/ocornut/imgui) - 即时模式图形界面库
  - [spdlog](https://github.com/gabime/spdlog) - 快速日志库
  - [entt](https://github.com/skypjack/entt) - 高性能实体组件系统
  - [glm](https://github.com/g-truc/glm) - 数学库
  - [stb](https://github.com/nothings/stb) - 图片加载库

## 项目结构

```
├── Fermion/          # 引擎核心代码
│   ├── Platform/     # 平台抽象层
│   │   ├── OpenGL/   # OpenGL 渲染实现
│   │   └── Window/   # 窗口系统实现
│   └── Sources/      # 源代码目录
│       ├── Core/     # 核心系统
│       ├── Debug/    # 调试系统
│       ├── Engine/   # 引擎主要功能
│       ├── Events/   # 事件系统
│       ├── ImGui/    # ImGui 集成
│       ├── Renderer/ # 渲染器组件
│       └── Time/     # 时间管理
├── external/         # 第三方依赖
│   ├── GLAD/         # OpenGL 加载库
│   ├── GLFW/         # 窗口和输入处理
│   ├── entt/         # 实体组件系统
│   ├── glm/          # 数学库
│   ├── imgui/        # ImGui 界面库
│   └── spdlog/       # 日志库
├── game/             # 示例游戏应用
├── assets/           # 游戏资源
│   └── textures/     # 纹理资源
├── doc/              # 文档
└── CMakeLists.txt    # 主构建文件
```

## 构建说明

### 前提条件

- CMake 3.16 或更高版本
- 支持 C++20 的编译器
  - Windows: Visual Studio 2019+
  - Linux: GCC 10+ 或 Clang 10+
  - macOS: Clang 11+

### 构建步骤

1. **克隆仓库**
   ```bash
   git clone https://github.com/Yang-Junjie/Fermion.git
   cd Fermion
   ```

2. **配置构建**
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. **编译项目**
   ```bash
   # Windows (Visual Studio)
   cmake --build . --config Release
   ```

4. **运行示例**
   编译后的可执行文件将位于 `bin/` 目录下

## 自定义构建选项

- `USE_OPENGL` - 是否使用 OpenGL 作为渲染后端（默认 ON）
  ```bash
  cmake .. -DUSE_OPENGL=OFF  
  ```
- `BUILD_EXAMPLES` - 是否构建示例应用（默认 ON）
  ```bash
  cmake .. -DBUILD_EXAMPLES=OFF  
  ```

## 使用示例

### 创建基本游戏应用

```cpp
#include "Engine/Engine.hpp"
#include "GameLayer.hpp"

namespace Fermion {
    class GameApp : public Engine {
    public:
        GameApp() {
            Log::Info("GameApp constructor called");
            pushLayer(std::make_unique<GameLayer>());
        }
        
        ~GameApp() = default;
    };
    
    // 引擎入口点 - 必须实现此函数
    Engine* createEngine() {
        Log::Info("start preparing to create the engine");
        return new Fermion::GameApp();
    }
}
```

### 创建游戏图层

```cpp
#include "Core/Layer.hpp"
#include "Core/Log.hpp"
#include "imgui.h"

namespace Fermion {
    class GameLayer : public Layer {
    public:
        GameLayer(const std::string& name = "GameLayer") : Layer(name) {
            // 图层初始化
        }
        
        virtual ~GameLayer() = default;
        
        // 当图层被添加到引擎时调用
        virtual void onAttach() override {
            // 初始化资源、设置
        }
        
        // 当图层从引擎移除时调用
        virtual void onDetach() override {
            // 清理资源
        }
        
        // 每帧更新逻辑
        virtual void onUpdate(Timestep dt) override {
            Log::Trace("GameLayer onUpdate called");
            // 游戏逻辑更新，dt为帧时间间隔
        }
        
        // 事件处理
        virtual void onEvent(IEvent& event) override {
            Log::Trace("GameLayer onEvent called: " + event.toString());
            // 处理键盘、鼠标等事件
        }
        
        // ImGui渲染
        virtual void onImGuiRender() override {
            ImGui::ShowDemoWindow();
            // 自定义ImGui界面
        }
    };
}
```

