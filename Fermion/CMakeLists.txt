project(engine LANGUAGES CXX)

set(Fermion_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Sources)
set(PLATFORM_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Platform)

add_library(engine STATIC
    ${Fermion_DIR}/fmpch.cpp
    ${Fermion_DIR}/Engine/Engine.cpp
    ${Fermion_DIR}/Core/Log.cpp
    ${Fermion_DIR}/Core/EntryPoint.cpp
    ${Fermion_DIR}/Core/LayerStack.cpp


    ${PLATFORM_DIR}/SFML/SFMLWindow.cpp
    ${PLATFORM_DIR}/SFML/SFMLRenderer.cpp
)

target_include_directories(engine PUBLIC
    ${Fermion_DIR}
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
    if(WIN32 AND BUILD_SHARED_LIBS)
        # 确保 bin 目录存在
        file(MAKE_DIRECTORY ${BIN_DIR})
        
        # 使用更可靠的方法查找SFML DLL
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            # 尝试不同可能的路径
            file(GLOB SFML_DLLS
                "${CMAKE_BINARY_DIR}/external/SFML/lib/Debug/sfml-*-d-3.dll"
                "${CMAKE_BINARY_DIR}/external/SFML/bin/sfml-*-d-3.dll"
                "${CMAKE_BINARY_DIR}/lib/Debug/sfml-*-d-3.dll"
                "${CMAKE_BINARY_DIR}/bin/sfml-*-d-3.dll"
            )
        else()
            # Release DLL
            file(GLOB SFML_DLLS
                "${CMAKE_BINARY_DIR}/external/SFML/lib/Release/sfml-*-3.dll"
                "${CMAKE_BINARY_DIR}/external/SFML/bin/sfml-*-3.dll"
                "${CMAKE_BINARY_DIR}/lib/Release/sfml-*-3.dll"
                "${CMAKE_BINARY_DIR}/bin/sfml-*-3.dll"
            )
        endif()

        # 拷贝 DLL 到 bin 目录
        foreach(dll ${SFML_DLLS})
            add_custom_command(TARGET engine POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${dll}
                ${BIN_DIR}/
                COMMENT "正在复制 ${dll} 到 ${BIN_DIR}/"
            )
        endforeach()
        
        # 为了确保可执行文件运行时能找到DLL，我们也为game目标添加DLL复制逻辑
        if(TARGET game)
            foreach(dll ${SFML_DLLS})
                add_custom_command(TARGET game POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${dll}
                    ${BIN_DIR}/
                    COMMENT "正在为game目标复制 ${dll} 到 ${BIN_DIR}/"
                )
            endforeach()
        endif()
    endif()
else()
    find_package(SDL2 REQUIRED)
    target_compile_definitions(engine PUBLIC USE_SDL_BACKEND)
    target_include_directories(engine PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(engine PUBLIC ${SDL2_LIBRARIES})
endif()
