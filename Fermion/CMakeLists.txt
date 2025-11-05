cmake_minimum_required(VERSION 3.16)
project(engine LANGUAGES CXX C)

set(FERMION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set(PLATFORM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Platform)
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

set(CORE_SOURCES
    ${FERMION_DIR}/fmpch.cpp
    ${FERMION_DIR}/Core/Engine.cpp

    ${FERMION_DIR}/Core/Log.cpp
    ${FERMION_DIR}/Core/EntryPoint.cpp
    ${FERMION_DIR}/Core/LayerStack.cpp
    ${FERMION_DIR}/Core/Window.cpp

    ${FERMION_DIR}/ImGui/ImGuiLayer.cpp

    ${FERMION_DIR}/Renderer/Renderer.cpp
    ${FERMION_DIR}/Renderer/RendererAPI.cpp
    ${FERMION_DIR}/Renderer/RenderCommand.cpp
    ${FERMION_DIR}/Renderer/GraphicsContext.cpp
    ${FERMION_DIR}/Renderer/Buffer.cpp
    ${FERMION_DIR}/Renderer/VertexArray.cpp
    ${FERMION_DIR}/Renderer/OrthographicCamera.cpp
    ${FERMION_DIR}/Renderer/OrthographicCameraController.cpp
    ${FERMION_DIR}/Renderer/Shader.cpp
    ${FERMION_DIR}/Renderer/Texture.cpp


)

set(PLATFORM_SOURCES
    ${PLATFORM_DIR}/OpenGL/OpenGLRendererAPI.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLContext.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLBuffer.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLVertexArray.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLTexture.cpp
    ${PLATFORM_DIR}/OpenGL/OpenGLShader.cpp
    
    ${PLATFORM_DIR}/Window/GLFWWindow.cpp
    ${PLATFORM_DIR}/Window/GLFWWindowInput.cpp

)

set(THIRD_PARTY_SOURCES
    ${EXTERNAL_DIR}/GLAD/src/glad.c

    ${EXTERNAL_DIR}/imgui/imgui.cpp
    ${EXTERNAL_DIR}/imgui/imgui_demo.cpp
    ${EXTERNAL_DIR}/imgui/imgui_draw.cpp
    ${EXTERNAL_DIR}/imgui/imgui_tables.cpp
    ${EXTERNAL_DIR}/imgui/imgui_widgets.cpp
    ${EXTERNAL_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${EXTERNAL_DIR}/imgui/backends/imgui_impl_opengl3.cpp

    ${EXTERNAL_DIR}/stb/stb_image.cpp
)

set(ENGINE_SOURCES
    ${CORE_SOURCES}
    ${PLATFORM_SOURCES}
    ${THIRD_PARTY_SOURCES}
)


add_library(engine STATIC ${ENGINE_SOURCES})


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
    ${EXTERNAL_DIR}/GLFW/include
    ${EXTERNAL_DIR}/stb
)


target_link_libraries(engine PUBLIC spdlog::spdlog)

if(USE_GLFW)
    target_link_libraries(engine PRIVATE glfw opengl32)
endif()
