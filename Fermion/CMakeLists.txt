project(engine LANGUAGES CXX C)

set(Fermion_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set(PLATFORM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Platform)


add_library(engine STATIC

    ${Fermion_DIR}/fmpch.cpp
    ${Fermion_DIR}/Engine/Engine.cpp
    ${Fermion_DIR}/Core/Log.cpp
    ${Fermion_DIR}/Core/EntryPoint.cpp
    ${Fermion_DIR}/Core/LayerStack.cpp
    ${Fermion_DIR}/Core/Window.cpp

    # ${Fermion_DIR}/ImGui/ImGuiLayer.cpp

    ${PLATFORM_DIR}/OpenGL/OpenGLRenderer.cpp
    ${PLATFORM_DIR}/Window/GLFWWindow.cpp
    ${CMAKE_SOURCE_DIR}/external/GLAD/src/glad.c
)



target_include_directories(engine PUBLIC
    ${Fermion_DIR}
    ${PLATFORM_DIR}/OpenGL
    ${PLATFORM_DIR}/Window
    ${CMAKE_SOURCE_DIR}/external/spdlog/include
    ${CMAKE_SOURCE_DIR}/external/entt/single_include
    ${CMAKE_SOURCE_DIR}/external/glm/glm
    ${CMAKE_SOURCE_DIR}/external/GLAD/include
    ${CMAKE_SOURCE_DIR}/external/GLFW/include

)

target_link_libraries(engine PUBLIC spdlog::spdlog)


if(USE_GLFW)
    target_link_libraries(engine PRIVATE glfw)
endif()