
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
            ${PLATFORM_DIR}/Windows/Utils/WindowsPlatformUtils.cpp
        )
    elseif(UNIX AND NOT APPLE)
        set(PLATFORM_EXCLUSIVE_SOURCES
            ${PLATFORM_DIR}/Linux/Utils/LinuxPlatformUtils.cpp)
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

    ${FERMION_DIR}/Renderer/Renderers/Renderer.cpp
    ${FERMION_DIR}/Renderer/RendererAPI.cpp
    ${FERMION_DIR}/Renderer/Renderers/Renderer2D.cpp
    ${FERMION_DIR}/Renderer/Batch/QuadBatch.cpp
    ${FERMION_DIR}/Renderer/Batch/CircleBatch.cpp
    ${FERMION_DIR}/Renderer/Batch/LineBatch.cpp
    ${FERMION_DIR}/Renderer/Batch/TextBatch.cpp
    ${FERMION_DIR}/Renderer/RenderCommandQueue.cpp
    ${FERMION_DIR}/Renderer/RenderGraph/RenderGraph.cpp
    ${FERMION_DIR}/Renderer/RenderGraph/RenderGraphResource.cpp
    ${FERMION_DIR}/Renderer/RenderGraph/RenderGraphResourcePool.cpp
    ${FERMION_DIR}/Renderer/RenderGraph/RenderGraphCompiler.cpp
    ${FERMION_DIR}/Renderer/RenderGraph/RenderGraphExecutor.cpp
    ${FERMION_DIR}/Renderer/RenderGraphLegacy.cpp
    ${FERMION_DIR}/Renderer/Pipeline.cpp
    ${FERMION_DIR}/Renderer/Renderers/DebugRenderer.cpp
    ${FERMION_DIR}/Renderer/GraphicsContext.cpp
    ${FERMION_DIR}/Renderer/Buffer.cpp
    ${FERMION_DIR}/Renderer/UniformBuffer.cpp
    ${FERMION_DIR}/Renderer/Framebuffer.cpp
    ${FERMION_DIR}/Renderer/VertexArray.cpp
    ${FERMION_DIR}/Renderer/Camera/OrthographicCamera.cpp
    ${FERMION_DIR}/Renderer/Camera/OrthographicCameraController.cpp
    ${FERMION_DIR}/Renderer/Camera/SceneCamera.cpp
    ${FERMION_DIR}/Renderer/Camera/EditorCamera.cpp
    ${FERMION_DIR}/Renderer/Renderers/SceneRenderer.cpp
    ${FERMION_DIR}/Renderer/Renderers/EnvironmentRenderer.cpp
    ${FERMION_DIR}/Renderer/Renderers/ShadowMapRenderer.cpp
    ${FERMION_DIR}/Renderer/Renderers/GBufferRenderer.cpp
    ${FERMION_DIR}/Renderer/Renderers/SSGIRenderer.cpp
    ${FERMION_DIR}/Renderer/Renderers/GTAORenderer.cpp
    ${FERMION_DIR}/Renderer/Renderers/DeferredLightingRenderer.cpp
    ${FERMION_DIR}/Renderer/Renderers/ForwardRenderer.cpp
    ${FERMION_DIR}/Renderer/Renderers/OutlineRenderer.cpp
    ${FERMION_DIR}/Renderer/Renderers/PostProcessRenderer.cpp
    ${FERMION_DIR}/Renderer/Renderers/ProceduralSkyGenerator.cpp
    ${FERMION_DIR}/Renderer/Renderers/InfiniteGridRenderer.cpp
    ${FERMION_DIR}/Renderer/Shader.cpp
    ${FERMION_DIR}/Renderer/Texture/Texture.cpp
    ${FERMION_DIR}/Renderer/Texture/SubTexture2D.cpp
    ${FERMION_DIR}/Renderer/Font/Font.cpp
    ${FERMION_DIR}/Renderer/Model/Mesh.cpp
    ${FERMION_DIR}/Renderer/Model/MeshFactory.cpp
    ${FERMION_DIR}/Renderer/Model/Material.cpp
    ${FERMION_DIR}/Renderer/Model/MaterialSerializer.cpp
    ${FERMION_DIR}/Renderer/Model/MaterialFactory.cpp
    ${FERMION_DIR}/Renderer/Model/MeshSerializer.cpp
    ${FERMION_DIR}/Renderer/Model/ModelSerializer.cpp
    ${FERMION_DIR}/Renderer/Preview/MaterialPreviewRenderer.cpp
    ${FERMION_DIR}/Renderer/Thumbnail/MaterialThumbnailProvider.cpp

    ${FERMION_DIR}/Project/Project.cpp
    ${FERMION_DIR}/Project/ProjectSerializer.cpp

    ${FERMION_DIR}/Scene/Scene.cpp
    ${FERMION_DIR}/Scene/Entity.cpp
    ${FERMION_DIR}/Scene/EntityManager.cpp
    ${FERMION_DIR}/Scene/SceneSerializer.cpp
    ${FERMION_DIR}/Physics/Physics3D.cpp
    ${FERMION_DIR}/Physics/Physics3DTypes.cpp
    ${FERMION_DIR}/Physics/Physics3DLayers.cpp
    ${FERMION_DIR}/Physics/Physics3DShapes.cpp
    ${FERMION_DIR}/Physics/Physics3DJoltInit.cpp

    ${FERMION_DIR}/Asset/AssetManager.cpp
    ${FERMION_DIR}/Asset/AssetRegistry.cpp
    ${FERMION_DIR}/Asset/Loader/TextureLoader.cpp
    ${FERMION_DIR}/Asset/Loader/FontLoader.cpp
    ${FERMION_DIR}/Asset/Loader/SceneLoader.cpp
    ${FERMION_DIR}/Asset/Loader/MeshLoader.cpp
    ${FERMION_DIR}/Asset/Loader/MaterialLoader.cpp
    ${FERMION_DIR}/Asset/Loader/ModelLoader.cpp
    ${FERMION_DIR}/Asset/Loader/SkeletonLoader.cpp
    ${FERMION_DIR}/Asset/Loader/AnimationClipLoader.cpp
    ${FERMION_DIR}/Asset/Importer/TextureImporter.cpp
    ${FERMION_DIR}/Asset/Importer/FontImporter.cpp
    ${FERMION_DIR}/Asset/Importer/SceneImporter.cpp
    ${FERMION_DIR}/Asset/Importer/ShaderImporter.cpp
    ${FERMION_DIR}/Asset/Importer/AssetImporter.cpp
    ${FERMION_DIR}/Asset/Importer/MeshImporter.cpp
    ${FERMION_DIR}/Asset/Importer/MaterialImporter.cpp
    ${FERMION_DIR}/Asset/Importer/ModelImporter.cpp
    ${FERMION_DIR}/Asset/Importer/ModelSourceImporter.cpp

    ${FERMION_DIR}/Animation/BoneTransform.cpp
    ${FERMION_DIR}/Animation/Skeleton.cpp
    ${FERMION_DIR}/Animation/AnimationClip.cpp
    ${FERMION_DIR}/Animation/AnimationSampler.cpp
    ${FERMION_DIR}/Animation/Animator.cpp
    ${FERMION_DIR}/Animation/SkeletonSerializer.cpp
    ${FERMION_DIR}/Animation/AnimationClipSerializer.cpp

    # ${FERMION_DIR}/Script/ScriptEngine.cpp
    ${FERMION_DIR}/Script/ScriptGlue.cpp
    ${FERMION_DIR}/Script/CSharp/CSharpScriptEngine.cpp


)

set(PLATFORM_SOURCES
    ${PLATFORM_DIR}/RenderApi/OpenGL/OpenGLRendererAPI.cpp
    ${PLATFORM_DIR}/RenderApi/OpenGL/OpenGLContext.cpp
    ${PLATFORM_DIR}/RenderApi/OpenGL/OpenGLBuffer.cpp
    ${PLATFORM_DIR}/RenderApi/OpenGL/OpenGLUniformBuffer.cpp
    ${PLATFORM_DIR}/RenderApi/OpenGL/OpenGLVertexArray.cpp
    ${PLATFORM_DIR}/RenderApi/OpenGL/OpenGLTexture.cpp
    ${PLATFORM_DIR}/RenderApi/OpenGL/OpenGLShader.cpp
    ${PLATFORM_DIR}/RenderApi/OpenGL/OpenGLPipeline.cpp
    ${PLATFORM_DIR}/RenderApi/OpenGL/OpenGLFramebuffer.cpp

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
file(GLOB IMGUI_NODE_EDITOR_SOURCES "${EXTERNAL_DIR}/ImguiNodeEditor/*.cpp")
set(IMGUI_NODE_EDITOR_UTILITIES_SOURCES
    ${EXTERNAL_DIR}/ImguiNodeEditor/examples/blueprints-example/utilities/drawing.cpp
    ${EXTERNAL_DIR}/ImguiNodeEditor/examples/blueprints-example/utilities/widgets.cpp
)
set(ENGINE_SOURCES
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
    ${THIRD_PARTY_SOURCES}
    ${YAML_CPP_SOURCES}
    ${IMGUIZMO_CPP_SOURCES}
    ${IMGUI_NODE_EDITOR_SOURCES}
    ${IMGUI_NODE_EDITOR_UTILITIES_SOURCES}
)


add_library(engine STATIC ${ENGINE_SOURCES})

# 添加Debug模式的编译定义
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(engine PUBLIC FERMION_DEBUG)
endif()


# 设置 Mono 库和包含目录
if(WIN32)
    add_library(Mono STATIC IMPORTED)
    set_target_properties(Mono PROPERTIES
        IMPORTED_LOCATION "${EXTERNAL_DIR}/mono/lib/libmono-static-sgen.lib"
    )
    set(MONO_LIB Mono)
    set(MONO_INCLUDE ${EXTERNAL_DIR}/mono/include)

    add_custom_command(
        TARGET engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${EXTERNAL_DIR}/mono/binary/mscorlib.dll
        ${BIN_DIR}/mscorlib.dll
    )
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MONO REQUIRED mono-2)

    set(MONO_LIB ${MONO_LIBRARIES})
    set(MONO_INCLUDE ${MONO_INCLUDE_DIRS})
endif()

target_include_directories(engine PUBLIC
    ${FERMION_DIR}

    ${PLATFORM_DIR}/RenderApi/OpenGL
    ${PLATFORM_DIR}/Windows/Utils
    ${PLATFORM_DIR}/Linux/Utils
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
    ${EXTERNAL_DIR}/ImguiNodeEditor
    ${EXTERNAL_DIR}/ImguiNodeEditor/examples/blueprints-example/utilities
    ${EXTERNAL_DIR}/box2d/include
    ${EXTERNAL_DIR}/JoltPhysics
    ${EXTERNAL_DIR}/msdf-atlas-gen/msdf-atlas-gen
    ${MONO_INCLUDE}
)



target_link_libraries(engine PUBLIC
    spdlog::spdlog
    box2d::box2d
    Jolt
    msdf-atlas-gen::msdf-atlas-gen
    ${MONO_LIB}
)



if(WIN32)
    target_link_libraries(engine PRIVATE
        ws2_32
        bcrypt
        version
        iphlpapi
        winmm
        opengl32
    )
elseif(UNIX)
    find_package(OpenGL REQUIRED)

    find_package(X11 REQUIRED)
    target_link_libraries(engine PRIVATE
        OpenGL::GL 
        ${X11_LIBRARIES}
    )
    target_include_directories(engine PRIVATE ${X11_INCLUDE_DIR})
endif()

if(USE_GLFW)
    target_compile_definitions(engine PRIVATE FM_PLATFORM_DESKTOP)

    target_link_libraries(engine PRIVATE glfw)
endif()

target_link_libraries(engine PUBLIC assimp)



