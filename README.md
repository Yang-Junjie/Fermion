# Fermion

Fermion 是一款基于 C++20 开发的轻量级游戏引擎，由 oxygen studio (Yang-Junjie)打造。引擎采用模块化设计，支持多渲染后端，提供简洁的 API 以简化游戏开发流程。



## 特性

- **跨平台支持**：通过抽象层支持多种渲染后端
- **模块化架构**：清晰的代码组织和分层设计
- **事件系统**：灵活的事件处理机制
- **图层系统**：基于 Layer 的场景管理
- **渲染功能**：支持基本图形绘制和图像渲染
- **日志系统**：集成 spdlog 提供详细日志
- **实体组件系统**：基于 entt 实现

## 技术栈与依赖

- **编程语言**：C++20
- **构建系统**：CMake 3.16+
- **主要依赖**：
  - [SFML](https://www.sfml-dev.org/) - 图形、窗口、音频库（默认后端）
  - [SDL2](https://www.libsdl.org/) - 可选渲染后端
  - [spdlog](https://github.com/gabime/spdlog) - 快速日志库
  - [entt](https://github.com/skypjack/entt) - 高性能实体组件系统
  - [glm](https://github.com/g-truc/glm) - 数学库

## 项目结构

```
├── Fermion/          # 引擎核心代码
│   ├── Platform/     # 平台抽象层
│   └── Sources/      # 源代码目录
├── external/         # 第三方依赖
│   ├── SFML/         # SFML 库
│   ├── entt/         # entt 实体组件系统
│   ├── glm/          # glm 数学库
│   └── spdlog/       # spdlog 日志库
├── game/             # 示例游戏应用
├── assets/           # 游戏资源
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
   
   # Linux/macOS
   cmake --build . -j$(nproc)
   ```

4. **运行示例**
   编译后的可执行文件将位于 `bin/` 目录下

## 自定义构建选项

- `USE_SFML` - 是否使用 SFML 作为渲染后端（默认 ON，暂时仅支持 SFML）,
  ```bash
  cmake .. -DUSE_SFML=OFF  
  ```

## 使用示例

### 创建基本游戏应用

```cpp
#include "Engine/Engine.hpp"

class MyGame : public Fermion::Engine {
public:
    MyGame() {
        // 构造函数
    }
    
    virtual void init() override {
        // 初始化游戏，添加图层
        pushLayer(std::make_unique<MyGameLayer>());
    }
};

// 引擎入口点
Fermion::Engine* createEngine() {
    return new MyGame();
}
```

### 创建游戏图层

```cpp
#include "Core/Layer.hpp"
#include "Core/Log.hpp"

class MyGameLayer : public Fermion::Layer {
public:
    MyGameLayer() : Layer("GameLayer") {}
    
    virtual void OnUpdate() override {
        // 每帧更新逻辑
        getRenderer()->drawRect({50, 50}, {200, 200}, {1.0f, 0.0f, 0.0f, 1.0f});
    }
    
    virtual void OnEvent(Fermion::IEvent& event) override {
        // 事件处理
        Fermion::Log::Trace("Event received: " + event.toString());
    }
};
```

