cmake_minimum_required(VERSION 3.16)
project(Fermion VERSION 0.1 LANGUAGES CXX C)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# option(BUILD_RELEASE "Build in Release mode" ON)


# if(BUILD_RELEASE)
#     set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
# else()
#     set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
# endif()



message("===== Your Platform =====")
if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    message("Platform: Android")
    set(PLATFORM "Android" CACHE STRING "Platform")
else()
    set(PLATFORM "Desktop" CACHE STRING "Platform")
    set(USE_GLFW ON CACHE BOOL "Use GLFW as window backend")
    if(WIN32)
        message("Platform: Windows")
        message("Use GLFW: ${USE_GLFW}")
    elseif(UNIX AND NOT APPLE)
        message("Platform: Linux")
        message("Use GLFW: ${USE_GLFW}")
    endif()
endif()
message("=========================")

# build boson
option(BUILD_BOSON "Build Boson" ON)


set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "Build yaml-cpp as static library" FORCE)

option(BUILD_NEUTRINO "Build Neutrino" ON)

# 这一行来解决yaml-cpp的dllimport问题
add_compile_definitions(YAML_CPP_STATIC_DEFINE)

# 输出目录
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin CACHE PATH "Output directory for binaries")

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
    add_subdirectory(Fermion/external/GLFW)
endif()

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

# 第三方库
add_subdirectory(Fermion/external/spdlog)
add_subdirectory(Fermion/external/entt)
add_subdirectory(Fermion/external/glm)
add_subdirectory(Fermion/external/yaml-cpp)
add_subdirectory(Fermion/external/box2d)
add_subdirectory(Fermion/external/assimp)
add_subdirectory(Fermion/external/freetype)
if(TARGET freetype AND NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

# msdf-atlas-gen (MSDF font atlas generator)
set(MSDF_ATLAS_BUILD_STANDALONE OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_USE_VCPKG OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_USE_SKIA OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_NO_ARTERY_FONT ON CACHE BOOL "" FORCE)
set(MSDF_ATLAS_DYNAMIC_RUNTIME ON CACHE BOOL "" FORCE)
set(MSDF_ATLAS_MSDFGEN_EXTERNAL OFF CACHE BOOL "" FORCE)
set(MSDF_ATLAS_INSTALL OFF CACHE BOOL "" FORCE)
set(MSDFGEN_DISABLE_PNG ON CACHE BOOL "Disable PNG dependency for msdfgen" FORCE)
add_subdirectory(Fermion/external/msdf-atlas-gen)


# Fermion 
add_subdirectory(Fermion)

# Boson
if(BUILD_BOSON)
    add_subdirectory(Boson)
    message("BUILD BOSON:${BUILD_BOSON}")
endif()

# Neutrino
if(BUILD_NEUTRINO)
    add_subdirectory(Neutrino)
    message("BUILD NEUTRINO:${BUILD_NEUTRINO}")
endif()

