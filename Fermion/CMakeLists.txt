
set(FERMION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set(PLATFORM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Platform)
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/Fermion/external)

if(PLATFORM STREQUAL "Desktop")
    set(PLATFORM_DEPENDENCIES_SOURCE
        ${PLATFORM_DIR}/Window/GLFWWindow.cpp
        ${PLATFORM_DIR}/Window/GLFWWindowInput.cpp
    )
    set(PLATFORM_DEPENDENCIES_INCLUDE_DIRS
        ${EXTERNAL_DIR}/GLFW/include
    )
    set(IMGUI_PLATFORM_SOURCE
        ${EXTERNAL_DIR}/imgui/backends/imgui_impl_glfw.cpp
    )
    if(WIN32)
        set(PLATFORM_EXCLUSIVE_SOURCES
            ${PLATFORM_DIR}/Window/WindowsPlatformUtils.cpp
        )
    endif()
elseif(Platform STREQUAL "Android")
    set(IMGUI_PLATFORM_SOURCE
        ${EXTERNAL_DIR}/imgui/backends/imgui_impl_android.cpp
    )
endif()


set(CORE_SOURCES
    ${FERMION_DIR}/fmpch.cpp
    ${FERMION_DIR}/Core/Application.cpp

    ${FERMION_DIR}/Core/Log.cpp
    ${FERMION_DIR}/Core/EntryPoint.cpp
    ${FERMION_DIR}/Core/LayerStack.cpp
    ${FERMION_DIR}/Core/Window.cpp
    ${FERMION_DIR}/Core/UUID.cpp

    ${FERMION_DIR}/ImGui/ImGuiLayer.cpp
    ${FERMION_DIR}/ImGui/ConsolePanel.cpp

    ${FERMION_DIR}/Math/Math.cpp

    ${FERMION_DIR}/Renderer/Renderer.cpp
    ${FERMION_DIR}/Renderer/RendererAPI.cpp
    ${FERMION_DIR}/Renderer/RenderCommand.cpp
    ${FERMION_DIR}/Renderer/Renderer2D.cpp
    ${FERMION_DIR}/Renderer/Renderer3D.cpp
    ${FERMION_DIR}/Renderer/CommandBuffer.cpp
    ${FERMION_DIR}/Renderer/RenderCommandQueue.cpp
    ${FERMION_DIR}/Renderer/RendererBackend.cpp
    ${FERMION_DIR}/Renderer/Pipeline.cpp
    ${FERMION_DIR}/Renderer/DebugRenderer.cpp
    ${FERMION_DIR}/Renderer/GraphicsContext.cpp
    ${FERMION_DIR}/Renderer/Buffer.cpp
    ${FERMION_DIR}/Renderer/FrameBuffer.cpp
    ${FERMION_DIR}/Renderer/VertexArray.cpp
    ${FERMION_DIR}/Renderer/Camera/OrthographicCamera.cpp
    ${FERMION_DIR}/Renderer/Camera/OrthographicCameraController.cpp
    ${FERMION_DIR}/Renderer/Camera/SceneCamera.cpp
    ${FERMION_DIR}/Renderer/Camera/EditorCamera.cpp
    ${FERMION_DIR}/Renderer/SceneRenderer.cpp
    ${FERMION_DIR}/Renderer/Shader.cpp
    ${FERMION_DIR}/Renderer/Texture/Texture.cpp
    ${FERMION_DIR}/Renderer/Texture/SubTexture2D.cpp
    ${FERMION_DIR}/Renderer/Font/Font.cpp
    ${FERMION_DIR}/Renderer/Model/Mesh.cpp
    ${FERMION_DIR}/Renderer/Model/MeshFactory.cpp
    ${FERMION_DIR}/Renderer/Model/Material.cpp

    ${FERMION_DIR}/Project/ProJect.cpp
    ${FERMION_DIR}/Project/ProjectSerializer.cpp

    ${FERMION_DIR}/Scene/Scene.cpp
    ${FERMION_DIR}/Scene/Entity.cpp
    ${FERMION_DIR}/Scene/SceneSerializer.cpp
    ${FERMION_DIR}/Physics/Physics3D.cpp

    ${FERMION_DIR}/Asset/AssetManager.cpp
    ${FERMION_DIR}/Asset/AssetRegistry.cpp
    ${FERMION_DIR}/Asset/Loader/TextureLoader.cpp
    ${FERMION_DIR}/Asset/Loader/FontLoader.cpp
    ${FERMION_DIR}/Asset/Loader/SceneLoader.cpp
    ${FERMION_DIR}/Asset/Loader/MeshLoader.cpp
    ${FERMION_DIR}/Asset/Importer/TextureImporter.cpp
    ${FERMION_DIR}/Asset/Importer/FontImporter.cpp
    ${FERMION_DIR}/Asset/Importer/SceneImporter.cpp
    ${FERMION_DIR}/Asset/Importer/ShaderImporter.cpp
    ${FERMION_DIR}/Asset/Importer/AssetImporter.cpp
    ${FERMION_DIR}/Asset/Importer/MeshImporter.cpp

    # ${FERMION_DIR}/Script/ScriptEngine.cpp
    ${FERMION_DIR}/Script/ScriptGlue.cpp
    ${FERMION_DIR}/Script/CSharp/CSharpScriptEngine.cpp


)

set(PLATFORM_SOURCES
    ${PLATFORM_DIR}/OpenGL/OpenGLRendererAPI.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLContext.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLBuffer.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLVertexArray.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLTexture.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLShader.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLPipeline.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLFrameBuffer.cpp

    ${PLATFORM_DEPENDENCIES_SOURCE}
    ${PLATFORM_EXCLUSIVE_SOURCES}

)

set(THIRD_PARTY_SOURCES
    ${EXTERNAL_DIR}/GLAD/src/glad.c

    ${EXTERNAL_DIR}/imgui/imgui.cpp
    ${EXTERNAL_DIR}/imgui/imgui_demo.cpp
    ${EXTERNAL_DIR}/imgui/imgui_draw.cpp
    ${EXTERNAL_DIR}/imgui/imgui_tables.cpp
    ${EXTERNAL_DIR}/imgui/imgui_widgets.cpp
    ${IMGUI_PLATFORM_SOURCE}
    ${EXTERNAL_DIR}/imgui/backends/imgui_impl_opengl3.cpp

    ${EXTERNAL_DIR}/stb/stb_image.cpp

)

file(GLOB YAML_CPP_SOURCES "${EXTERNAL_DIR}/yaml-cpp/src/*.cpp")
file(GLOB IMGUIZMO_CPP_SOURCES "${EXTERNAL_DIR}/ImGuizmo/*.cpp")
set(ENGINE_SOURCES
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
    ${THIRD_PARTY_SOURCES}
    ${YAML_CPP_SOURCES}
    ${IMGUIZMO_CPP_SOURCES}
)


add_library(engine STATIC ${ENGINE_SOURCES})

# 添加Debug模式的编译定义
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(engine PUBLIC FERMION_DEBUG)
endif()

add_custom_command(
    TARGET engine POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy # 使用 CMake 内置命令 copy
    ${EXTERNAL_DIR}/mono/binary/mscorlib.dll
    ${BIN_DIR}/mscorlib.dll
)

target_include_directories(engine PUBLIC
    ${FERMION_DIR}

    ${PLATFORM_DIR}/OpenGL
    ${PLATFORM_DIR}/Window


    ${EXTERNAL_DIR}/spdlog/include
    ${EXTERNAL_DIR}/entt/single_include
    ${EXTERNAL_DIR}/glm
    ${EXTERNAL_DIR}/imgui
    ${EXTERNAL_DIR}/imgui/backends
    ${EXTERNAL_DIR}/GLAD/include
    ${PLATFORM_DEPENDENCIES_INCLUDE_DIRS}
    ${EXTERNAL_DIR}/stb
    ${EXTERNAL_DIR}/yaml-cpp/include
    ${EXTERNAL_DIR}/ImGuizmo
    ${EXTERNAL_DIR}/box2d/include
    ${EXTERNAL_DIR}/JoltPhysics
    ${EXTERNAL_DIR}/msdf-atlas-gen/msdf-atlas-gen
    # ${EXTERNAL_DIR}/box2d/src
    ${EXTERNAL_DIR}/mono/include
)

add_library(Mono STATIC IMPORTED)

set_target_properties(Mono PROPERTIES
    IMPORTED_LOCATION "${EXTERNAL_DIR}/mono/lib/libmono-static-sgen.lib"
)

target_link_libraries(engine PUBLIC
    spdlog::spdlog
    box2d::box2d
    Jolt
    msdf-atlas-gen::msdf-atlas-gen
    Mono
    ws2_32
    bcrypt
    version
    iphlpapi
    winmm
)


if(USE_GLFW)
    target_compile_definitions(engine PRIVATE FM_PLATFORM_DESKTOP)
    target_link_libraries(engine PRIVATE glfw)
endif()


target_link_libraries(engine PRIVATE opengl32)
target_link_libraries(engine PUBLIC assimp)



