cmake_minimum_required(VERSION 3.16)
project(Fermion LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_SFML "Use SFML as rendering backend" ON)
option(USE_GLFW "Use GLFW as rendering backend" OFF)

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

message(STATUS "CMake构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "输出目录: ${BIN_DIR}")
message(STATUS "SFML使用共享库: ${BUILD_SHARED_LIBS}")

if(USE_SFML)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    add_subdirectory(external/SFML)
    # ImGui-SFML配置
    set(IMGUI_SFML_FIND_SFML OFF)
    set(IMGUI_SFML_IMGUI_DEMO OFF)
    set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui") # 告诉imgui-sfml ImGui的位置
    add_subdirectory(external/imgui-sfml)
elseif(USE_GLFW)
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
add_subdirectory(game)

# 添加一个辅助函数来复制DLL（如果需要手动复制）
if(WIN32 AND USE_SFML AND BUILD_SHARED_LIBS)
    add_custom_target(copy_sfml_dlls
        COMMENT "复制SFML DLL文件到${BIN_DIR}"
    )

    if(TARGET game)
        add_dependencies(game copy_sfml_dlls)
    endif()

    message(STATUS "构建完成后，请检查${BIN_DIR}目录是否包含所有必要的SFML DLL文件")
endif()


