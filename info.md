[main] 正在生成文件夹: f:/Beisent/Fermion/build all
[build] 正在启动生成
[proc] 正在执行命令: H:\CMake\bin\cmake.EXE --build f:/Beisent/Fermion/build --config Debug --target all --
[build] [1/2   0% :: 0.132] Re-checking globbed directories...
[build] [7/12   8% :: 3.480] Building CXX object Boson\CMakeFiles\BosonEditor.dir\src\Panels\InspectorPanel.cpp.obj
[build] FAILED: Boson/CMakeFiles/BosonEditor.dir/src/Panels/InspectorPanel.cpp.obj 
[build] "H:\Visual Studio\Community\VC\Tools\MSVC\14.50.35717\bin\Hostx64\x64\cl.exe"  /nologo /TP -DFERMION_DEBUG -DJPH_DEBUG_RENDERER -DJPH_FLOATING_POINT_EXCEPTIONS_ENABLED -DJPH_OBJECT_STREAM -DJPH_PROFILE_ENABLED -DMSDFGEN_COPYRIGHT_YEAR=2026 -DMSDFGEN_DISABLE_PNG -DMSDFGEN_DISABLE_SVG -DMSDFGEN_EXTENSIONS -DMSDFGEN_EXT_PUBLIC="" -DMSDFGEN_PUBLIC="" -DMSDFGEN_USE_CPP11 -DMSDFGEN_VERSION=1.12.1 -DMSDFGEN_VERSION_MAJOR=1 -DMSDFGEN_VERSION_MINOR=12 -DMSDFGEN_VERSION_REVISION=1 -DMSDF_ATLAS_COPYRIGHT_YEAR=2026 -DMSDF_ATLAS_NO_ARTERY_FONT -DMSDF_ATLAS_PUBLIC="" -DMSDF_ATLAS_VERSION=1.3.0 -DMSDF_ATLAS_VERSION_MAJOR=1 -DMSDF_ATLAS_VERSION_MINOR=3 -DMSDF_ATLAS_VERSION_REVISION=0 -DSPDLOG_COMPILED_LIB -DYAML_CPP_STATIC_DEFINE -IF:\Beisent\Fermion\Boson -IF:\Beisent\Fermion\Fermion\external\ImGuizmo -IF:\Beisent\Fermion\HilbertSpace -IF:\Beisent\Fermion\Fermion\Sources -IF:\Beisent\Fermion\Fermion\Platform\RenderApi\OpenGL -IF:\Beisent\Fermion\Fermion\Platform\Windows\Utils -IF:\Beisent\Fermion\Fermion\Platform\Linux\Utils -IF:\Beisent\Fermion\Fermion\Platform\Window -IF:\Beisent\Fermion\Fermion\external\spdlog\include -IF:\Beisent\Fermion\Fermion\external\entt\single_include -IF:\Beisent\Fermion\Fermion\external\glm -IF:\Beisent\Fermion\Fermion\external\imgui -IF:\Beisent\Fermion\Fermion\external\imgui\backends -IF:\Beisent\Fermion\Fermion\external\GLAD\include -IF:\Beisent\Fermion\Fermion\external\GLFW\include -IF:\Beisent\Fermion\Fermion\external\stb -IF:\Beisent\Fermion\Fermion\external\yaml-cpp\include -IF:\Beisent\Fermion\Fermion\external\ImguiNodeEditor -IF:\Beisent\Fermion\Fermion\external\ImguiNodeEditor\examples\blueprints-example\utilities -IF:\Beisent\Fermion\Fermion\external\box2d\include -IF:\Beisent\Fermion\Fermion\external\JoltPhysics -IF:\Beisent\Fermion\Fermion\external\msdf-atlas-gen\msdf-atlas-gen -IF:\Beisent\Fermion\Fermion\external\mono\include -IF:\Beisent\Fermion\Fermion\external\box2d\src\..\include -IF:\Beisent\Fermion\build\Fermion\external\box2d\src -IF:\Beisent\Fermion\Fermion\external\JoltPhysics\Build\.. -IF:\Beisent\Fermion\Fermion\external\msdf-atlas-gen -IF:\Beisent\Fermion\Fermion\external\msdf-atlas-gen\msdfgen -IF:\Beisent\Fermion\Fermion\external\assimp\code\..\include -IF:\Beisent\Fermion\build\Fermion\external\assimp\code\..\include /DWIN32 /D_WINDOWS /GR /EHsc /Zi /Ob0 /Od /RTC1 -std:c++20 -MDd /utf-8 /showIncludes /FoBoson\CMakeFiles\BosonEditor.dir\src\Panels\InspectorPanel.cpp.obj /FdBoson\CMakeFiles\BosonEditor.dir\ /FS -c F:\Beisent\Fermion\Boson\src\Panels\InspectorPanel.cpp
[build] F:\Beisent\Fermion\Boson\src\Panels\InspectorPanel.cpp(701): error C2662: “Fermion::UUID Fermion::Entity::getUUID(void)”: 不能将“this”指针从“const Fermion::Entity”转换为“Fermion::Entity &”
[build] F:\Beisent\Fermion\Boson\src\Panels\InspectorPanel.cpp(701): note: 转换丢失限定符
[build] F:\Beisent\Fermion\Fermion\Sources\Scene/Entity.hpp(138): note: 参见“Fermion::Entity::getUUID”的声明
[build] F:\Beisent\Fermion\Boson\src\Panels\InspectorPanel.cpp(701): note: 尝试匹配参数列表“()”时
[build] F:\Beisent\Fermion\Boson\src\Panels\InspectorPanel.cpp(701): note: 模板实例化上下文(最早的实例化上下文)为
[build] F:\Beisent\Fermion\Boson\src\Panels\InspectorPanel.cpp(569): note: 查看对正在编译的函数 模板 实例化“void Fermion::drawComponent<Fermion::ScriptContainerComponent,Fermion::InspectorPanel::drawComponents::<lambda_9>>(const std::string &,Fermion::Entity,UIFunction)”的引用
[build]         with
[build]         [
[build]             UIFunction=Fermion::InspectorPanel::drawComponents::<lambda_9>
[build]         ]
[build] F:\Beisent\Fermion\Boson\src\Panels\InspectorPanel.cpp(315): note: 查看对正在编译的函数 模板 实例化“auto Fermion::InspectorPanel::drawComponents::<lambda_9>::operator ()<T>(_T1 &) const”的引用
[build]         with
[build]         [
[build]             T=Fermion::ScriptContainerComponent,
[build]             _T1=Fermion::ScriptContainerComponent
[build]         ]
[build] [7/12  16% :: 4.604] Building CXX object Fermion\CMakeFiles\engine.dir\Sources\Project\Project.cpp.obj
[build] [7/12  25% :: 4.820] Building CXX object Fermion\CMakeFiles\engine.dir\Sources\Core\Application.cpp.obj
[build] [7/12  33% :: 4.877] Building CXX object Fermion\CMakeFiles\engine.dir\Sources\Script\CSharp\CSharpScriptEngine.cpp.obj
[build] [7/12  41% :: 5.862] Building CXX object Boson\CMakeFiles\BosonEditor.dir\src\BosonLayer.cpp.obj
[build] [7/12  50% :: 6.884] Building CXX object Fermion\CMakeFiles\engine.dir\Sources\Scene\Scene.cpp.obj
[build] [7/12  58% :: 7.139] Building CXX object Fermion\CMakeFiles\engine.dir\Sources\Script\ScriptGlue.cpp.obj
[build] ninja: build stopped: subcommand failed.
[proc] 命令“H:\CMake\bin\cmake.EXE --build f:/Beisent/Fermion/build --config Debug --target all --”已退出，代码为 1
[driver] 生成完毕: 00:00:07.191
[build] 生成已完成，退出代码为 1
[main] 无法准备名称为 "undefined" 的可执行目标