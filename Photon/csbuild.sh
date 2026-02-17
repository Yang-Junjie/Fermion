#!/bin/bash
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
