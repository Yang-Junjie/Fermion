project(engine LANGUAGES CXX)

set(OXYGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set(PLATFORM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Platform)

add_library(engine STATIC
    ${OXYGINE_DIR}/Engine/Engine.cpp
    ${OXYGINE_DIR}/Core/Log.cpp
    ${PLATFORM_DIR}/SFML/SFMLWindow.cpp
    ${PLATFORM_DIR}/SFML/SFMLRenderer.cpp

)

target_include_directories(engine PUBLIC
    ${OXYGINE_DIR}
    ${PLATFORM_DIR}/SFML
    ${CMAKE_SOURCE_DIR}/external/spdlog/include
    ${CMAKE_SOURCE_DIR}/external/entt/single_include
    ${CMAKE_SOURCE_DIR}/external/glm/glm
)


target_link_libraries(engine PUBLIC spdlog::spdlog)




if(WIN32 AND BUILD_SHARED_LIBS)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(SPDLOG_DLL ${CMAKE_BINARY_DIR}/external/spdlog/spdlogd.dll)
    else()
        set(SPDLOG_DLL ${CMAKE_BINARY_DIR}/external/spdlog/spdlog.dll)
    endif()

    add_custom_command(TARGET engine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SPDLOG_DLL} ${BIN_DIR}/
    )
endif()

# 链接 SFML
if(USE_SFML)
    target_compile_definitions(engine PUBLIC USE_SFML_BACKEND)
    target_link_libraries(engine PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
    )
    # Windows 下的 DLL
    if(WIN32 AND USE_SFML AND BUILD_SHARED_LIBS)
        # Debug DLL 名称
        set(SFML_DLL_DIR ${CMAKE_BINARY_DIR}/external/SFML/lib)
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            file(GLOB SFML_DLLS
                "${SFML_DLL_DIR}/sfml-graphics-d-2.dll"
                "${SFML_DLL_DIR}/sfml-window-d-2.dll"
                "${SFML_DLL_DIR}/sfml-system-d-2.dll"
                "${SFML_DLL_DIR}/sfml-audio-d-2.dll"
                "${SFML_DLL_DIR}/sfml-network-d-2.dll"
            )
        else()
            # Release DLL 名称
            file(GLOB SFML_DLLS
                "${SFML_DLL_DIR}/sfml-graphics-2.dll"
                "${SFML_DLL_DIR}/sfml-window-2.dll"
                "${SFML_DLL_DIR}/sfml-system-2.dll"
                "${SFML_DLL_DIR}/sfml-audio-2.dll"
                "${SFML_DLL_DIR}/sfml-network-2.dll"
            )
        endif()

        # 拷贝 DLL 到 bin 目录
        foreach(dll ${SFML_DLLS})
            add_custom_command(TARGET engine POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${dll}
                ${BIN_DIR}/
            )
        endforeach()
    endif()
else()
    find_package(SDL2 REQUIRED)
    target_compile_definitions(engine PUBLIC USE_SDL_BACKEND)
    target_include_directories(engine PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(engine PUBLIC ${SDL2_LIBRARIES})
endif()
