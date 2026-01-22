#!/bin/bash

# 确保脚本在出错时停止
set -e

echo "开始编译 Photon.dll..."

# 调用 mcs 编译器
mcs -unsafe -out:Photon.dll -target:library \
    ../Photon/Vector3.cs \
    ../Photon/Scene/Entity.cs \
    ../Photon/Scene/Scene.cs \
    ../Photon/Renderer/DebugRenderer.cs \
    ../Photon/InternalCalls.cs \
    ../Photon/Scene/Components.cs \
    ../Photon/Vector2.cs \
    ../Photon/Vector4.cs \
    ../Photon/KeyCode.cs \
    ../Photon/Utils.cs \
    ../Photon/Input.cs

echo "编译成功: Photon.dll"
