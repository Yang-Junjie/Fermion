cmake_minimum_required(VERSION 3.16)
project(OxyGine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_SFML "Use SFML as rendering backend" ON)

# 输出目录
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)

if(USE_SFML)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    add_subdirectory(external/SFML)
endif()

add_subdirectory(engine)
add_subdirectory(game)
