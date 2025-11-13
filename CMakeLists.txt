cmake_minimum_required(VERSION 3.16)
project(Fermion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# option(USE_SFML "Use SFML as rendering backend" OFF)
option(USE_GLFW "Use GLFW as rendering backend" ON)

# build boson
option(BUILD_BOSON "Build Boson" ON)
# option(BUILD_GAME "Build Fermion" OFF)

# build game
# option(BUILD_BOSON "Build Boson" OFF)
option(BUILD_GAME "Build Fermion" ON)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "Build yaml-cpp as static library" FORCE)
add_subdirectory(external/yaml-cpp)

# 添加这一行来解决yaml-cpp的dllimport问题
add_compile_definitions(YAML_CPP_STATIC_DEFINE)

# 输出目录
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)

# 确保输出目录存在
file(MAKE_DIRECTORY ${BIN_DIR})

# 设置运行时输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${BIN_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${BIN_DIR})

if(USE_GLFW)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    add_subdirectory(external/GLFW)
endif()

# 第三方库
add_subdirectory(external/spdlog)
add_subdirectory(external/entt)
add_subdirectory(external/glm)

# Fermion 
add_subdirectory(Fermion)

if(BUILD_GAME)
add_subdirectory(game)
endif()

# Boson
if(BUILD_BOSON)
add_subdirectory(Boson)
endif()