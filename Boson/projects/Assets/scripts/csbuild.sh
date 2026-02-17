#!/bin/bash
mcs -out:test.dll -target:library \
-r:Photon.dll \
../Boson/projects/Assets/scripts/CameraController.cs \
../Boson/projects/Assets/scripts/PressureTest.cs \
../Boson/projects/Assets/scripts/XAxisMove.cs \
../Boson/projects/Assets/scripts/Rigidbody3DController.cs \
../Boson/projects/Assets/scripts/FPSCameraController.cs \
../Boson/projects/Assets/scripts/FPSCharacterController.cs \
../Boson/projects/Assets/scripts/SnakeGame.cs \
../Boson/projects/Assets/scripts/CubePhysicsDemo.cs \
../Boson/projects/Assets/scripts/CharacterController2D.cs \
../Boson/projects/Assets/scripts/XAxisMove2D.cs \
../Boson/projects/Assets/scripts/CircleSensorTest.cs
